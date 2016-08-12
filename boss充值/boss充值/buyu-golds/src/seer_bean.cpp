//-----------------------------------//
//-----------------------------------//

#include <unistd.h>
#include <map>
#include <utility>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <arpa/inet.h>

#include "benchapi.h"
#include "seer_bean_proto.h"
#include "ini_file.h"
#include "db_connection_mgr.h"
#include "mq_mgr.h"
#include "mysql_res_auto_ptr.h"
#include "my_exception.h"
#include "seer_bean_stat.h"
#include "util.h"

//-----------------------------------//
//-----------------------------------//

typedef struct {
    int  channel_id;
    int  channel_type;
    char sz_security_code[8 + 1];
    int  init_trans_id;
} __attribute__((packed)) channel_info_t;

typedef std::map<int,channel_info_t> channel_map_t;
typedef channel_map_t::const_iterator channel_map_const_iterator_t;
typedef channel_map_t::iterator channel_map_iterator_t;

//-----------------------------------//
//-----------------------------------//

char g_sz_query_sql[1024 * 4];
char g_sz_send_buffer[1024 * 4];

// 同步协议发送的命令号
int g_sync_cmd_id = 0;

//-----------------------------------//
//-----------------------------------//

c_db_connection_mgr_t g_db_connection_mgr;
c_mq_mgr_t g_mq_mgr;
channel_map_t g_channel_map;
c_seer_bean_stat_t g_statistic_mgr;

//-----------------------------------//
//-----------------------------------//

template<typename type_name_t> type_name_t min(type_name_t left,type_name_t right)
{
    if(left <= right)
    {
        return left;
    }
    else
    {
        return right;
    }
}

//-----------------------------------//
//-----------------------------------//

int init_channel_map()
{
    //-------------------
    //clear the channel map first
    g_channel_map.clear();

    //-------------------
    //form query sql
    memset(g_sz_query_sql,0,sizeof(g_sz_query_sql));
    sprintf(g_sz_query_sql,"SELECT channel_id,channel_type,security_code,init_trans_id FROM seer_bean_channel_table WHERE status = 0");

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(0,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //query
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return -1;
    }

    //-------------------
    //get result
    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return -1;
    }

    //-------------------
    //get all channel info
    mysql_row_t data_row;
    while((data_row = mysql_fetch_row(p_record_set)))
    {
        channel_info_t channel_info;
        memset(&channel_info,0,sizeof(channel_info));

        channel_info.channel_id = atoi(data_row[0]);
        channel_info.channel_type = atoi(data_row[1]);

        int copy_len = min(strlen(data_row[2]),sizeof(channel_info.sz_security_code) - 1);
        strncpy(channel_info.sz_security_code,data_row[2],copy_len);

        channel_info.init_trans_id = atoi(data_row[3]);

        //-------------------
        //verify data
        if(channel_info.channel_id <= 0)
        {
            continue;
        }

        //-------------------
        //insert into map
        g_channel_map.insert(std::make_pair(channel_info.channel_id,channel_info));
    }

    //-------------------
    //check channel count
    if(g_channel_map.size() <= 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//

int verify_channel(seer_bean_proto_verify_header_t& verify_header,uint8_t* p_data,int data_len, uint32_t* init_trans_id = NULL)
{
    //-------------------
    //param check
    if(NULL == p_data || data_len <= 0 || data_len >= 1024)
    {
        return -1;
    }

    int channel_id = verify_header.channel_id;

    //-------------------
    //find the channel info
    channel_map_const_iterator_t iterator = g_channel_map.find(channel_id);
    if(iterator == g_channel_map.end())
    {
        //-------------------
        //can not find the channel
        return -1;
    }

    //-------------------
    //generate md5 code
    char sz_temp_buffer[1024 * 4];
    sprintf(sz_temp_buffer,"channelId=%d&securityCode=%s&data=",channel_id,iterator->second.sz_security_code);

    int total_len = strlen(sz_temp_buffer);
    memcpy(sz_temp_buffer + total_len,p_data,data_len);
    total_len += data_len;

    uint8_t raw_md5[16 + 1];
    MD5((unsigned char*)sz_temp_buffer,total_len,raw_md5);

    char sz_md5[32 + 1];
    memset(sz_md5,0,sizeof(sz_md5));
    asc_to_hex_lowercase(sz_md5,(char*)raw_md5,16);
    assert(strlen(sz_md5) == 32);

    //-------------------
    //get md5 code
    char sz_original_md5[33];
    memset(sz_original_md5,0,sizeof(sz_original_md5));
    strncpy(sz_original_md5,(char*)verify_header.sz_md5,32);

    //-------------------
    //compare md5 code
    if(strcmp(sz_md5,sz_original_md5))
    {
        //-------------------
        //two md5 is not same,invalid data
        return -1;
    }
    else
    {
        if (init_trans_id != NULL)
        {
            *init_trans_id = iterator->second.init_trans_id;
        }
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//

//DBProxy 设置金豆协议,cmd_id: 0x19a6
typedef struct {
	uint32_t proto_length;
	uint32_t proto_id;
	uint16_t cmd_id;
	uint32_t result;
	uint32_t user_id;
	uint32_t bean_num; //金豆数，1金豆则bean_num=100
    uint32_t costs;
    uint32_t item_id;      //购买的商品id
    uint32_t third_channel_id;
} __attribute__((packed)) dbproxy_set_bean_t;

/**
 * @brief
 *
 * @param user_id  用户米米号
 * @param bean_num 用户金豆数，1金豆则bean_num=100
 *
 * @return 0 on success, -1 on error
 */
int sync_bean_balance(uint32_t user_id, uint32_t bean_num, uint32_t costs, uint32_t item_id, uint32_t third_channel_id)
{
	//-------------------
	//form package
	dbproxy_set_bean_t bean_set;
	memset(&bean_set, 0, sizeof(bean_set));

	bean_set.proto_length = sizeof(bean_set);
	bean_set.cmd_id = g_sync_cmd_id;
	bean_set.user_id = user_id;
	bean_set.bean_num = bean_num;
    bean_set.costs = costs;
    bean_set.item_id = item_id;
    bean_set.third_channel_id = third_channel_id;

	//-------------------
	//send to message queue
	int result = g_mq_mgr.send_msg_timedout(&bean_set, sizeof(bean_set), 1 ,8); //wait for 8 seconds
	if (result)
	{
		//-------------------
		//send data failed
		ERROR_LOG("sync bean failed! user_id=%u, bean_num=%u", user_id, bean_num);
		return -1;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------//
//-----------------------------------//
//return : 0 - success  > 0 - failed,error code

int create_account(int account_id)
{
    //-------------------
    //param check
    if(account_id <= 0)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(account_id,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //check whether the account exists
    sprintf(g_sz_query_sql,"SELECT account_id FROM seer_bean_account_table WHERE account_id = %d",account_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set);

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    if(mysql_num_rows(p_record_set) > 0)
    {
        return PACK_E_ACCOUNT_ALREADY_EXISTS;
    }

    //-------------------
    //create account
    sprintf(g_sz_query_sql,"INSERT INTO seer_bean_account_table(account_id,seer_bean_num) VALUES(%d,%d)",account_id,0);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        //-------------------
        //create account fail
        if (mysql_errno(p_db_connection) == ER_DUP_ENTRY)
        {
            //用户记录已存在
            return PACK_E_ACCOUNT_ALREADY_EXISTS;
        }
        else
        {
            ERROR_LOG("db operation failed! account_id = %d",account_id);
            return PACK_E_OPERATION_FAILED;
        }
    }
    else
    {
        //-------------------
        //create account success
        return PACK_S_OK;
    }
}

//-----------------------------------//
//-----------------------------------//
//return 0 if sucess or error code

int add_seer_bean(int channel_id,int account_id,int seer_bean_num, int costs, int item_id, int external_trans_id, int third_channel_id, uint64_t* p_transaction_id)
{
    //-------------------
    //param check
    if(account_id <= 0 || seer_bean_num <= 0)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(account_id,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //check whether the account exists
    sprintf(g_sz_query_sql, "SELECT seer_bean_num FROM seer_bean_account_table WHERE account_id = %d", account_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    if(mysql_num_rows(p_record_set) <= 0)
    {
        //-------------------
        // seer_bean account does not exist, create it
        int create_result = create_account(account_id);
        if (create_result != PACK_S_OK && create_result != PACK_E_ACCOUNT_ALREADY_EXISTS)
        {
            //-------------------
            //create account failed
            ERROR_LOG("auto create account failed! account_id = %d",account_id);
            return PACK_E_OPERATION_FAILED;
        }
    }

    //-------------------
    //add seer_bean,do db operation
    try
    {
        //-------------------
        //begin db transaction
        if(mysql_autocommit(p_db_connection,false))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //lock the account record
        sprintf(g_sz_query_sql, "SELECT account_id FROM seer_bean_account_table WHERE account_id = %d FOR UPDATE", account_id);
        if (mysql_real_query(p_db_connection, g_sz_query_sql, strlen(g_sz_query_sql)))
        {
            ERROR_LOG("mysql_real_query() failed, [SQL:%s]", g_sz_query_sql);
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
        }

        mysql_res_t* p_lock_record_set = mysql_store_result(p_db_connection);
        c_mysql_res_auto_ptr_t lock_res_auto_ptr(p_lock_record_set); //auto ptr

        if (NULL == p_lock_record_set)
        {
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
        }

        if (mysql_num_rows(p_lock_record_set) <= 0)
        {
            //-------------------
            //account not exists, should not appear!
            ERROR_LOG("account record not exist, account_id=%d", account_id);
            throw c_my_exception_t(mysql_errno(p_db_connection), "account record not exist");
        }

        //-------------------
        //check whether the channel and the external_trans_id already exist
        sprintf(g_sz_query_sql, "SELECT transaction_id FROM seer_bean_transaction_table WHERE channel_id = %d AND external_trans_id = %d", channel_id, external_trans_id);
        if (mysql_real_query(p_db_connection, g_sz_query_sql, strlen(g_sz_query_sql)))
        {
            ERROR_LOG("mysql_real_query(), [SQL:%s]", g_sz_query_sql);
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
        }

        mysql_res_t* p_second_record_set = mysql_store_result(p_db_connection);
        c_mysql_res_auto_ptr_t second_res_auto_ptr(p_second_record_set); //second auto ptr

        if (NULL == p_second_record_set)
        {
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
        }

        if (mysql_num_rows(p_second_record_set) > 0)
        {
            //-------------------
            //the transaction has been dealt,should return the transaction id
            mysql_row_t data_row = mysql_fetch_row(p_second_record_set);

            if (data_row && p_transaction_id)
            {
                *p_transaction_id = atoi(data_row[0]);
            }

            //-------------------
            //commit db transaction
            if (mysql_commit(p_db_connection))
            {
                throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
            }

            //-------------------
            //restore db auto commit
            mysql_autocommit(p_db_connection, true);

            return PACK_E_TRANS_ALREADY_DEALED;
        }

        //-------------------
        //update  seer_bean account
        sprintf(g_sz_query_sql,"UPDATE seer_bean_account_table SET seer_bean_num = seer_bean_num + %d, costs = costs + %d WHERE account_id = %d",seer_bean_num, costs, account_id);
        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //get new  seer_bean account balance
        sprintf(g_sz_query_sql,"SELECT seer_bean_num, costs FROM seer_bean_account_table WHERE account_id = %d",account_id);
        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        mysql_res_t* p_third_record_set = mysql_store_result(p_db_connection);
        c_mysql_res_auto_ptr_t third_res_auto_ptr(p_third_record_set); //third auto ptr

        if(NULL == p_third_record_set)
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        mysql_row_t data_row = mysql_fetch_row(p_third_record_set);
        if(NULL == data_row)
        {
            //-------------------
            //unexpected error
            throw c_my_exception_t(0,"add  seer_bean unexpected error!");
        }

        int seer_bean_balance = atoi(data_row[0]);
        assert(seer_bean_balance > 0);
        
        int costs_balance = atoi(data_row[1]);
        assert(costs_balance > 0);

        //-------------------
        //write transaction log
        sprintf(g_sz_query_sql,"INSERT INTO seer_bean_transaction_table(account_id,dest_account_id,channel_id,external_trans_id,seer_bean_num,seer_bean_balance) \
            VALUES(%d,%d,%d,%d,%d,%d)",account_id,account_id,channel_id,external_trans_id,seer_bean_num,seer_bean_balance);

        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        uint64_t transaction_id = mysql_insert_id(p_db_connection);

        //-------------------
        //commit db transaction
        if(mysql_commit(p_db_connection))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //restore db auto commit
        mysql_autocommit(p_db_connection,true);

        //-------------------
        //add  seer_bean statistic
        g_statistic_mgr.add_seer_bean(account_id,seer_bean_num,channel_id);

        //-------------------
        //sync user bean balance to DBProxy
        sync_bean_balance((uint32_t)account_id, (uint32_t)seer_bean_balance, costs_balance, item_id, third_channel_id);

        //-------------------
        //return transaction id
        if(p_transaction_id)
        {
            *p_transaction_id = transaction_id;
        }

        return PACK_S_OK;
    }
    catch(c_my_exception_t& exception)
    {
        //-------------------
        //catch exception
        mysql_rollback(p_db_connection);
        mysql_autocommit(p_db_connection,true);

        ERROR_LOG("add  seer_bean db operation failed! channel_id = %d,account_id = %d,seer_bean_num = %d,external_trans_id = %d,error info = %s",
            channel_id,account_id,seer_bean_num,external_trans_id,exception.get_exception_message());

        return PACK_E_OPERATION_FAILED;
    }
}

//-----------------------------------//
//-----------------------------------//
//0 if success or error code if failed

int consume_seer_bean(int channel_id,int account_id,int dest_account_id,int product_id,int product_count,int seer_bean_to_consume,int external_trans_id,uint64_t* p_transaction_id,uint32_t* p_seer_bean_balance)
{
    //-------------------
    //param check
    if(account_id <= 0 || dest_account_id <= 0 || seer_bean_to_consume <= 0 || product_id <= 0 || product_count <= 0)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(account_id,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //query seer_bean account balance
    sprintf(g_sz_query_sql,"SELECT seer_bean_num,is_allow_transfer,status FROM seer_bean_account_table WHERE account_id = %d",account_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set);

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_row_t data_row = mysql_fetch_row(p_record_set);
    if(NULL == data_row)
    {
        return PACK_E_ACCOUNT_NOT_EXISTS;
    }

    int seer_bean_balance = atoi(data_row[0]);

    if(seer_bean_balance < seer_bean_to_consume)
    {
        return PACK_E_NOT_ENOUGH_SEER_BEAN;
    }

    //-------------------
    //check the account_id and dest_account_id (consume for self ?)
    if(account_id != dest_account_id)
    {
        bool is_allow_transfer = atoi(data_row[1]);
        if(!is_allow_transfer)
        {
            //-------------------
            //not allow to pay for others
            return PACK_E_NOT_ALLOW_TRANSFER;
        }
    }

    //-------------------
    //check account status
    int account_status = atoi(data_row[2]);
    if(account_status != 0)
    {
        return PACK_E_ACCOUNT_LOCKED;
    }

    //-------------------
    //check whether the channel and the external_trans_id already exist
    sprintf(g_sz_query_sql,"SELECT transaction_id FROM seer_bean_transaction_table WHERE channel_id = %d AND external_trans_id = %d",channel_id,external_trans_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
    	ERROR_LOG("mysql_real_query() failed, [SQL:%s]", g_sz_query_sql);
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_second_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t second_res_auto_ptr(p_second_record_set); //second auto ptr

    if(NULL == p_second_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    if(mysql_num_rows(p_second_record_set) > 0)
    {
        //-------------------
        //the transaction has been dealed,should return the transaction id
        mysql_row_t data_row = mysql_fetch_row(p_second_record_set);

        if(data_row && p_transaction_id)
        {
            *p_transaction_id = atoi(data_row[0]);
        }

        return PACK_E_TRANS_ALREADY_DEALED;
    }

    //-------------------
    //consume  seer_bean,do db operation
    try
    {
        //-------------------
        //begin db transaction
        if(mysql_autocommit(p_db_connection,false))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //update  account
        sprintf(g_sz_query_sql,"UPDATE seer_bean_account_table SET seer_bean_num = seer_bean_num - %d WHERE account_id = %d AND seer_bean_num >= %d",seer_bean_to_consume,account_id,seer_bean_to_consume);
        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }
        else
        {
            //-------------------
            //get affected rows
            if(mysql_affected_rows(p_db_connection) <= 0)
            {
                //-------------------
                //not enough  seer_bean
                ERROR_LOG("race condition lead to not enough  seer_bean! account_id = %d",account_id);
                throw c_my_exception_t(PACK_E_NOT_ENOUGH_SEER_BEAN,"not enough  seer_bean");
            }
        }

        //-------------------
        //get new  seer_bean balance
        sprintf(g_sz_query_sql,"SELECT seer_bean_num FROM seer_bean_account_table WHERE account_id = %d",account_id);
        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        mysql_res_t* p_third_record_set = mysql_store_result(p_db_connection);
        c_mysql_res_auto_ptr_t third_res_auto_ptr(p_third_record_set); //third auto ptr

        if(NULL == p_third_record_set)
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        mysql_row_t data_row = mysql_fetch_row(p_third_record_set);
        if(NULL == data_row)
        {
            //-------------------
            //unexpected error
            throw c_my_exception_t(0,"consume  seer_bean unexpected error!");
        }

        int seer_bean_balance = atoi(data_row[0]);
        //assert(seer_bean_balance >= 0);

        //-------------------
        //write transaction log
        sprintf(g_sz_query_sql,"INSERT INTO seer_bean_transaction_table(account_id,dest_account_id,channel_id,external_trans_id,product_id,product_count,seer_bean_num,seer_bean_balance) \
            VALUES(%d,%d,%d,%d,%d,%d,%d,%d)",account_id,dest_account_id,channel_id,external_trans_id,product_id,product_count,-seer_bean_to_consume,seer_bean_balance);

        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        uint64_t transaction_id = mysql_insert_id(p_db_connection);

        //-------------------
        //commit db transaction
        if(mysql_commit(p_db_connection))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //restore db auto commit
        mysql_autocommit(p_db_connection,true);

        //-------------------
        //consume  seer_bean statistic
        g_statistic_mgr.consume_seer_bean(account_id,seer_bean_to_consume,channel_id,product_id,product_count);

		//-------------------
		//sync user bean balance to DBProxy
        int cost = 0, item_id = 0, third_channel_id = 0;
		sync_bean_balance((uint32_t)account_id, (uint32_t)seer_bean_balance, cost, item_id, third_channel_id);

        //-------------------
        //return transaction id and  num balance
        if(p_transaction_id)
        {
            *p_transaction_id = transaction_id;
        }

        if(p_seer_bean_balance)
        {
            *p_seer_bean_balance = seer_bean_balance;
        }

        return PACK_S_OK;
    }
    catch(c_my_exception_t& exception)
    {
        //-------------------
        //catch exception
        mysql_rollback(p_db_connection);
        mysql_autocommit(p_db_connection,true);

        ERROR_LOG("consume  seer_bean db operation failed! channel_id = %d,account_id = %d,seer_bean_to_consume = %d,product_id = %d,external_trans_id = %d,error info = %s",
            channel_id,account_id,seer_bean_to_consume,product_id,external_trans_id,exception.get_exception_message());

        if(exception.get_exception_code() == PACK_E_NOT_ENOUGH_SEER_BEAN)
        {
            return PACK_E_NOT_ENOUGH_SEER_BEAN;
        }
        else
        {
            return PACK_E_OPERATION_FAILED;
        }
    }//catch
}

//-----------------------------------//
//-----------------------------------//
//return 0 if success or error code if failed

int get_seer_bean_balance(int account_id,int* p_seer_bean_balance,int* p_account_status)
{
    //-------------------
    //param check
    if(account_id <= 0)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(account_id,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //query account balance
    sprintf(g_sz_query_sql,"SELECT seer_bean_num,status FROM seer_bean_account_table WHERE account_id = %d",account_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_row_t data_row = mysql_fetch_row(p_record_set);
    if(data_row == NULL)
    {
        return PACK_E_ACCOUNT_NOT_EXISTS;
    }

    if(p_seer_bean_balance)
    {
        *p_seer_bean_balance = atoi(data_row[0]);
    }

    if(p_account_status)
    {
        *p_account_status = atoi(data_row[1]);
    }

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int get_trans_info(uint64_t transaction_id,seer_bean_proto_query_trans_ack_t* p_proto_ack)
{
    //-------------------
    //param check
    if(transaction_id <= 0 || NULL == p_proto_ack)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(0,&p_db_connection);
    assert(p_db_connection != NULL);
    // 由于分表扩容，需要判断交易号是否小于最小的交易号，以区分交易号不存在和交易号在历史表中的情形
    sprintf(g_sz_query_sql,"SELECT min(transaction_id) FROM seer_bean_transaction_table");
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr_query(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_row_t data_row = mysql_fetch_row(p_record_set);
    if(NULL == data_row)
    {
        return PACK_E_OPERATION_FAILED;
    }

    //-------------------
    //return
    if (transaction_id < (uint32_t)atoi(data_row[0])) {
        return PACK_E_TRANS_SMALLER_THAN_INIT;
    }

    res_auto_ptr_query.free();

    //-------------------
    //query transaction status
    sprintf(g_sz_query_sql,"SELECT account_id,dest_account_id,UNIX_TIMESTAMP(transaction_time),channel_id,external_trans_id,product_id,product_count,seer_bean_num FROM seer_bean_transaction_table WHERE transaction_id = %u",(uint32_t)transaction_id);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    data_row = mysql_fetch_row(p_record_set);
    if(NULL == data_row)
    {
        return PACK_E_INVALID_TRANS_ID;;
    }

    //-------------------
    //return
    p_proto_ack->account_id = atoi(data_row[0]);
    p_proto_ack->dest_account_id = atoi(data_row[1]);
    p_proto_ack->transaction_time = atoi(data_row[2]);
    p_proto_ack->channel_id = atoi(data_row[3]);
    p_proto_ack->external_trans_id = atoi(data_row[4]);
    p_proto_ack->product_id = atoi(data_row[5]);
    p_proto_ack->product_count = atoi(data_row[6]);
    p_proto_ack->seer_bean_num = atoi(data_row[7]);

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int query_account_history(int account_id,int start_time,int end_time,int start_row_index,int end_row_index,seer_bean_proto_query_account_history_ack_t* p_proto_ack)
{
    //-------------------
    //param check
    if(account_id <= 0 || start_time < 0 || end_time < start_time || start_row_index < 0 || end_row_index <= start_row_index)
    {
        return PACK_E_INVALID_PARAM;
    }

    if(NULL == p_proto_ack)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(0,&p_db_connection);
    assert(p_db_connection != NULL);

    //-------------------
    //query total row count
    sprintf(g_sz_query_sql,"SELECT COUNT(*) AS total_row_count FROM seer_bean_transaction_table WHERE account_id = %d AND transaction_time >= FROM_UNIXTIME(%d) AND transaction_time <= FROM_UNIXTIME(%d)",account_id,start_time,end_time);
    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set); //auto ptr

    if(NULL == p_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_row_t data_row = mysql_fetch_row(p_record_set);
    if(NULL == data_row)
    {
        return PACK_E_ACCOUNT_NOT_EXISTS;
    }

    int total_row_count = atoi(data_row[0]);

    //-------------------
    //query history rows
    sprintf(g_sz_query_sql,"SELECT UNIX_TIMESTAMP(transaction_time),channel_id,seer_bean_num,seer_bean_balance,product_id,product_count \
        FROM seer_bean_transaction_table WHERE account_id = %d AND transaction_time >= FROM_UNIXTIME(%d) AND transaction_time <= FROM_UNIXTIME(%d) \
        ORDER BY transaction_time DESC,transaction_id DESC LIMIT %d,%d",account_id,start_time,end_time,start_row_index,end_row_index - start_row_index);

    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        return PACK_E_OPERATION_FAILED;
    }

    mysql_res_t* p_second_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t second_res_auto_ptr(p_second_record_set); //second auto ptr

    if(NULL == p_second_record_set)
    {
        return PACK_E_OPERATION_FAILED;
    }

    int max_data_len = sizeof(p_proto_ack->row_array) / sizeof(seer_bean_proto_account_history_row_t);
    int data_len = 0;

    while( (data_row = mysql_fetch_row(p_second_record_set)) )
    {
        if(data_len >= max_data_len)
        {
            break;
        }

        p_proto_ack->row_array[data_len].transaction_time = atoi(data_row[0]);
        p_proto_ack->row_array[data_len].channel_id = atoi(data_row[1]);
        p_proto_ack->row_array[data_len].seer_bean_num = atoi(data_row[2]);
        p_proto_ack->row_array[data_len].seer_bean_balance = atoi(data_row[3]);
        p_proto_ack->row_array[data_len].product_id = atoi(data_row[4]);
        p_proto_ack->row_array[data_len].product_count = atoi(data_row[5]);

        data_len++;
    }

    p_proto_ack->total_row_count = total_row_count;
    p_proto_ack->current_row_count = data_len;

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int print_package_log(unsigned int remote_ip,int cmd_id,int channel_id,int account_id,char* p_package_data,int data_len)
{
    //-------------------
    //param check
    if(NULL == p_package_data || data_len < 1)
    {
        return -1;
    }

    if(data_len >= 1024)
    {
        data_len = 1024 -1;
    }

    //-------------------
    //form package log
    char sz_package_buffer[1024 * 3 + 1];
    memset(sz_package_buffer,0,sizeof(sz_package_buffer));
    int package_buffer_data_len = 0;

    for(int index = 0; index < data_len; index++)
    {
        int hex = ((unsigned char)p_package_data[index]) >> 4;
        sz_package_buffer[package_buffer_data_len++] = hex < 10 ? '0' + hex : 'a' - 10 + hex;

        hex = ((unsigned char)p_package_data[index]) & 0x0F;
        sz_package_buffer[package_buffer_data_len++] = hex < 10 ? '0' + hex : 'a' - 10 + hex;

        sz_package_buffer[package_buffer_data_len++] = ' ';
    }

    //-------------------
    //print package log
    in_addr remote_addr;
    remote_addr.s_addr = remote_ip;

    INFO_LOG("[ip=%s][cmd_id=%d][data_len=%d][channel_id=%d][account_id=%d][%s]",inet_ntoa(remote_addr),cmd_id,data_len,channel_id,account_id,sz_package_buffer);

    return 0;
}

//-----------------------------------//
//-----------------------------------//

extern "C" int handle_init(int argc,char* argv[],int pid_type)
{
    //-------------------
    //init work proc
    if(pid_type == PROC_WORK)
    {
        //-------------------
        //read config file
        const char* lpsz_config_file = "../etc/buyu_golds.conf";

        //db
        char sz_db_ip[20];
        memset(sz_db_ip,0,sizeof(sz_db_ip));

        char sz_db_user[50];
        memset(sz_db_user,0,sizeof(sz_db_user));

        char sz_db_pwd[50];
        memset(sz_db_pwd,0,sizeof(sz_db_pwd));

        char sz_db_name[50];
        memset(sz_db_name,0,sizeof(sz_db_name));

        read_profile_string("mysql","db_ip",sz_db_ip,sizeof(sz_db_ip),"0.0.0.0",lpsz_config_file);
        read_profile_string("mysql","db_user",sz_db_user,sizeof(sz_db_user),"unknown",lpsz_config_file);
        read_profile_string("mysql","db_pwd",sz_db_pwd,sizeof(sz_db_pwd),"unknown",lpsz_config_file);
        read_profile_string("mysql","db_name",sz_db_name,sizeof(sz_db_name),"unknown",lpsz_config_file);

        int db_port = read_profile_int("mysql","db_port",3306,lpsz_config_file);

        //mq
        char sz_mq_name[100] = { 0 };
        read_profile_string("mq", "name", sz_mq_name, sizeof(sz_mq_name), "", lpsz_config_file);

        int msg_max = read_profile_int("mq","msg_max",0,lpsz_config_file);
        int msgsize_max = read_profile_int("mq","msgsize_max",0,lpsz_config_file);

        //stat
        char sz_statistic_file_full_path[512];
        memset(sz_statistic_file_full_path,0,sizeof(sz_statistic_file_full_path));
        read_profile_string("statistic","file_full_path",sz_statistic_file_full_path,sizeof(sz_statistic_file_full_path),"",lpsz_config_file);

        //-------------------
        //init db connection mgr
        if(g_db_connection_mgr.init(sz_db_ip,sz_db_user,sz_db_pwd,sz_db_name,db_port))
        {
            ERROR_LOG("init db connection mgr failed! check mysql db params!");
            return -1;
        }

        //-------------------
        //init mq mgr
        if( g_mq_mgr.init(sz_mq_name, 0777, msg_max, msgsize_max) ) //todo: 0777->0700?
        {
            ERROR_LOG("init mq failed! check mq params!");
            return -1;
        }

        //-------------------
        //init channel map
        if(init_channel_map())
        {
            ERROR_LOG("init seer_bean channel map failed! check _channel_table!");
            g_db_connection_mgr.uninit();

            return -1;
        }
        
        g_sync_cmd_id = read_profile_int("sync_cmd","cmd_id",50150,lpsz_config_file);

        //-------------------
        //init statistic mgr
        if(g_statistic_mgr.init(sz_statistic_file_full_path))
        {
            ERROR_LOG("init statistic mgr failed!");
            g_db_connection_mgr.uninit();
            g_channel_map.clear();

            return -1;
        }

        return 0;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//

extern "C" int handle_input(const char* p_buffer, int length, const skinfo_t* p_skinfo)
{
    //-------------------
    //get package length
    if(length < 4)
    {
        //-------------------
        //continue to recv
        return 0;
    }

    uint32_t package_length = 0;
    memcpy(&package_length,p_buffer,sizeof(package_length));

    if(package_length < sizeof(seer_bean_proto_header_t) || package_length > 1024)
    {
        return -1;
    }
    else
    {
        return package_length;
    }
}

//-----------------------------------//
//-----------------------------------//

extern "C" int handle_process(char* p_recv_buffer,int recv_len,char** pp_send_buffer, int* p_send_len, const skinfo_t* p_skinfo)
{
    //-------------------
    //check param
    if(recv_len < (int)sizeof(seer_bean_proto_header_t))
    {
        return -1;
    }

    //-------------------
    //get proto header
    seer_bean_proto_header_t proto_header;
    memcpy(&proto_header,p_recv_buffer,sizeof(proto_header));

    if((int)proto_header.package_len != recv_len)
    {
        return -1;
    }

    //-------------------
    //deal package
    proto_header.status_code = PACK_E_UNKNOWN;
    int cmd_id = proto_header.command_id;

    if(SEER_BEAN_PROTO_CREATE_ACCOUNT_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t))
        {
            return -1;
        }

        //-------------------
        //deal package
        do
        {
            //-------------------
            //try to create account
            proto_header.status_code = create_account(proto_header.account_id);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,0,proto_header.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header);
        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(SEER_BEAN_PROTO_ADD_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t) + sizeof(seer_bean_proto_add_t))
        {
            return -1;
        }

        //-------------------
        //deal add  package
        seer_bean_proto_add_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(seer_bean_proto_header_t),sizeof(proto_body));

        seer_bean_proto_add_ack_t proto_ack;

        do
        {
            //-------------------
            //verify channel
            uint32_t init_trans_id = 0;
            if(verify_channel(proto_body.verify_header,proto_body.verify_header.data,sizeof(proto_body) - sizeof(proto_body.verify_header),&init_trans_id))
            {
                proto_header.status_code = PACK_E_INVALID_CHANNEL;
                break;
            }

            // 由于分表扩容后，需要判断渠道交易号是否比当前渠道的其实交易号还要小
            if (proto_body.external_trans_id <= init_trans_id)
            {
                proto_header.status_code = PACK_E_TRANS_SMALLER_THAN_INIT;
                break;
            }

            //-------------------
            //add  seer_bean
            proto_header.status_code = add_seer_bean(proto_body.verify_header.channel_id,proto_header.account_id,proto_body.seer_bean_num,  proto_body.costs, proto_body.item_id, proto_body.external_trans_id, proto_body.third_channel_id, &proto_ack.transaction_id);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,proto_body.verify_header.channel_id,proto_header.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(SEER_BEAN_PROTO_CONSUME_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t) + sizeof(seer_bean_proto_consume_t))
        {
            return -1;
        }

        //-------------------
        //deal consume  seer_bean package
        seer_bean_proto_consume_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(seer_bean_proto_header_t),sizeof(proto_body));

        seer_bean_proto_consume_ack_t proto_ack;

        do
        {
            //-------------------
            //verify channel
            uint32_t init_trans_id = 0;
            if(verify_channel(proto_body.verify_header,proto_body.verify_header.data,sizeof(proto_body) - sizeof(proto_body.verify_header),&init_trans_id))
            {
                proto_header.status_code = PACK_E_INVALID_CHANNEL;
                break;
            }

            // 由于分表扩容后，需要判断渠道交易号是否比当前渠道的起始交易号还要小
            if (proto_body.external_trans_id <= init_trans_id)
            {
                proto_header.status_code = PACK_E_TRANS_SMALLER_THAN_INIT;
                break;
            }

            //-------------------
            //consume
            proto_header.status_code = consume_seer_bean(proto_body.verify_header.channel_id,proto_header.account_id,proto_body.dest_account_id,proto_body.product_id,
                proto_body.product_count,proto_body.seer_bean_num,proto_body.external_trans_id,&proto_ack.transaction_id,&proto_ack.seer_bean_balance);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,proto_body.verify_header.channel_id,proto_header.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(SEER_BEAN_PROTO_TRANSFER_CMD_ID == cmd_id)
    {
        return -1;
    }
    else if(SEER_BEAN_PROTO_QUERY_BALANCE_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t))
        {
            return -1;
        }

        //-------------------
        //deal query  balance package
        seer_bean_proto_query_balance_ack_t proto_ack;

        do
        {
            //-------------------
            //query balance and account status
            proto_header.status_code = get_seer_bean_balance(proto_header.account_id,(int*)&proto_ack.seer_bean_balance,(int*)&proto_ack.account_status);

        }while(0);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(SEER_BEAN_PROTO_QUERY_TRANS_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t) + sizeof(seer_bean_proto_query_trans_t))
        {
            return -1;
        }

        //-------------------
        //deal query transaction package
        seer_bean_proto_query_trans_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(seer_bean_proto_header_t),sizeof(proto_body));

        seer_bean_proto_query_trans_ack_t proto_ack;

        do
        {
            //-------------------
            //query trans info
            proto_header.status_code = get_trans_info(proto_body.transaction_id,&proto_ack);

        }while(0);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(SEER_BEAN_PROTO_QUERY_ACCOUNT_HISTORY_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(seer_bean_proto_header_t) + sizeof(seer_bean_proto_query_account_history_t))
        {
            return -1;
        }

        //-------------------
        //deal query account history package
        seer_bean_proto_query_account_history_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(seer_bean_proto_header_t),sizeof(proto_body));

        seer_bean_proto_query_account_history_ack_t proto_ack;

        do
        {
            //-------------------
            //query account history
            proto_header.status_code = query_account_history(proto_header.account_id,proto_body.start_time,proto_body.end_time,
                proto_body.start_row_index,proto_body.end_row_index,&proto_ack);

        }while(0);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else
    {
        return -1;
    }
}

//-----------------------------------//
//-----------------------------------//

extern "C" void handle_fini(int pid_type)
{
    if(pid_type == PROC_WORK)
    {
        g_db_connection_mgr.uninit();
        g_mq_mgr.uninit();
        g_channel_map.clear();
    }
}

//-----------------------------------//
//-----------------------------------//
