//-----------------------------------//
//-----------------------------------//

#include <unistd.h>
#include <map>
#include <utility>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>
#include <mysql/mysqld_error.h>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

#include "benchapi.h"
#include "mb_proto.h"
#include "ini_file.h"
#include "db_connection_mgr.h"
#include "mysql_res_auto_ptr.h"
#include "my_exception.h"
#include "mb_statistic.h"
#include "util.h"
#include "proto.xml.h"
#include "cfg_file_parse.h"

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

// 各游戏每月消费上限的数据结构
typedef struct consume_limit_entry {
    int start;
    int end;
    int limit;

    bool operator<(const consume_limit_entry &m) const
    {
        return start < m.start;
    }
} consume_limit_entry_t;
vector<consume_limit_entry_t> g_consume_limit_list;

// 渠道统计项的数据结构
map<uint32_t, uint32_t> g_channel_stat_map;

//-----------------------------------//
//-----------------------------------//

char g_sz_query_sql[1024 * 4];
char g_sz_send_buffer[1024 * 4];

//-----------------------------------//
//-----------------------------------//

c_db_connection_mgr_t g_db_connection_mgr;
channel_map_t g_channel_map;
c_mb_statistic_t g_statistic_mgr;

//-----------------------------------//
//-----------------------------------//

int g_mole_item_monthly_consume_limit = 0; //mole商品购买月消费最大值
int g_seer_item_monthly_consume_limit = 0; //seer商品购买月消费最大值
int g_ddt_item_monthly_consume_limit = 0; //dandantang商品购买月消费最大值
int g_haqi_item_monthly_consume_limit = 0; //haqi商品购买月消费最大值
int g_gongfu_item_monthly_consume_limit = 0; //功夫派商品购买月消费最大值
int g_baobei_item_monthly_consume_limit = 0; //宝贝大战商品购买月消费最大值
int g_xxrenzhe_item_monthly_consume_limit = 0; //宝贝大战商品购买月消费最大值

//-----------------------------------//
//-----------------------------------//

std::vector<uint32_t> g_allow_ips; //允许IP地址列表

//-----------------------------------//
//-----------------------------------//

template<typename type_name_t> type_name_t chnl_min(type_name_t left,type_name_t right)
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

int init_ip_list(std::vector<uint32_t>& ip_list)
{
	const char* config_file = "../etc/security.conf";
	const char* section = "ip_list";

	int ip_count = read_profile_int(section, "ip_count", -1, config_file);
	if (ip_count < 0)
	{
		ERROR_LOG("key [ip_count] in section [%s] in file [%s] is invalid", section, config_file);
		return -1;
	}

	char key[16] = { 0 };
	char str_ip[16] = { 0 };
	uint32_t ip = 0;
	struct in_addr ia_ip;

	for (int i=1; i<=ip_count; i++)
	{
		sprintf(key, "ip%d", i);
		read_profile_string( section, key, str_ip, sizeof(str_ip), "none", config_file);

		if ( strlen(str_ip)>15 || strcmp(str_ip,"none")==0 )
		{
			ERROR_LOG("key [%s] in section [%s] in file [%s] is invalid", key, section, config_file);
			return -1;
		}

		if (inet_aton(str_ip, &ia_ip))
		{
			ip = (uint32_t)ia_ip.s_addr;
			ip_list.push_back(ip);
		}
		else
		{
			ERROR_LOG("key [%s] in section [%s] in file [%s] is invalid", key, section, config_file);
			return -1;
		}
	}

	return 0;
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
    sprintf(g_sz_query_sql,"SELECT channel_id,channel_type,security_code,init_trans_id FROM channel_table WHERE status = 0");

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
        channel_info.init_trans_id = atoi(data_row[3]);

        int copy_len = chnl_min(strlen(data_row[2]),sizeof(channel_info.sz_security_code) - 1);
        strncpy(channel_info.sz_security_code,data_row[2],copy_len);

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

void asc_to_hex_lowercase(char* sz_dst, char* sz_src,int len)
{
    int hex = 0;
    int index = 0;
    int dest_index = 0;

    for(index = 0; index < len; index++)
    {
        hex = ((unsigned char)sz_src[index]) >> 4;
        dest_index = index * 2;
        sz_dst[dest_index] = hex < 10 ? '0' + hex : 'a' - 10 + hex ;

        hex = ((unsigned char)sz_src[index]) & 0x0F;
        sz_dst[dest_index + 1] = hex < 10 ? '0' + hex : 'a'- 10 + hex ;
    }

    sz_dst[len * 2] = 0;
}

//-----------------------------------//
//-----------------------------------//

int verify_channel(mb_proto_verify_header_t& verify_header,uint8_t* p_data,int data_len, uint32_t* init_trans_id = NULL)
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
    sprintf(g_sz_query_sql,"SELECT account_id FROM account_table WHERE account_id = %d",account_id);
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
    sprintf(g_sz_query_sql,"INSERT INTO account_table(account_id,mb_num) VALUES(%d,%d)",account_id,0);
    if (mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
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

int add_mb(int channel_id,int account_id,int mb_num,int pay_gate_trans_id,int private_data_01,char* p_private_data_02,uint64_t* p_transaction_id)
{
    //-------------------
    //param check
    if(account_id <= 0 || mb_num <= 0 || NULL == p_private_data_02)
    {
        return PACK_E_INVALID_PARAM;
    }

    if(strlen(p_private_data_02) >= 128)
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
    sprintf(g_sz_query_sql,"SELECT account_id FROM account_table WHERE account_id = %d",account_id);
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

    if (mysql_num_rows(p_record_set) <= 0)
    {
        //-------------------
        //mb account not exists,create it
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
    //add mb,do db operation
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
        sprintf(g_sz_query_sql, "SELECT account_id FROM account_table WHERE account_id = %d FOR UPDATE", account_id);
        if (mysql_real_query(p_db_connection, g_sz_query_sql, strlen(g_sz_query_sql)))
        {
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
            //mb account not exists, should not appear!
            ERROR_LOG("account record not exist, account_id=%d", account_id);
            throw c_my_exception_t(mysql_errno(p_db_connection), "account record not exist");
        }

        //-------------------
        //check if the channel and the pay_gate_trans_id already exist
        sprintf(g_sz_query_sql, "SELECT transaction_id FROM transaction_table WHERE channel_id = %d AND pay_gate_trans_id = %d", channel_id, pay_gate_trans_id);
        if (mysql_real_query(p_db_connection, g_sz_query_sql, strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
        }

        mysql_res_t* p_second_record_set = mysql_store_result(p_db_connection);
        c_mysql_res_auto_ptr_t second_res_auto_ptr(p_second_record_set); //second auto ptr

        if(NULL == p_second_record_set)
        {
            throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
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

            //-------------------
            //commit db transaction
            if(mysql_commit(p_db_connection))
            {
                throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
            }

            //-------------------
            //restore db auto commit
            mysql_autocommit(p_db_connection,true);

            return PACK_E_TRANS_ALREADY_DEALED;
        }

        //-------------------
        //update mb account
        sprintf(g_sz_query_sql,"UPDATE account_table SET mb_num = mb_num + %d WHERE account_id = %d",mb_num,account_id);
        if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //get new mb account balance
        sprintf(g_sz_query_sql,"SELECT mb_num FROM account_table WHERE account_id = %d",account_id);
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
            throw c_my_exception_t(0,"add mb unexpected error!");
        }

        int mb_account_balance = atoi(data_row[0]);
        assert(mb_account_balance > 0);

        //-------------------
        //write transaction log
        char sz_private_data_02_escaped[256 + 1];
        memset(sz_private_data_02_escaped,0,sizeof(sz_private_data_02_escaped));
        mysql_real_escape_string(p_db_connection,sz_private_data_02_escaped,p_private_data_02,strlen(p_private_data_02));

        sprintf(g_sz_query_sql,"INSERT INTO transaction_table(account_id,dest_account_id,channel_id,pay_gate_trans_id,mb_num,mb_account_balance,private_data_01,private_data_02) \
            VALUES(%d,%d,%d,%d,%d,%d,%d,'%s')",account_id,account_id,channel_id,pay_gate_trans_id,mb_num,mb_account_balance,private_data_01,sz_private_data_02_escaped);

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
        //add mb statistic
        g_statistic_mgr.add_mb(account_id,mb_num,channel_id);

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

        ERROR_LOG("add mb db operation failed! channel_id = %d,account_id = %d,mb_num = %d,pay_gate_trans_id = %d,error info = %s",
            channel_id,account_id,mb_num,pay_gate_trans_id,exception.get_exception_message());

        return PACK_E_OPERATION_FAILED;
    }
}

//-----------------------------------//
//-----------------------------------//
//return monthly_consumed mb if success or -1 if failed

int get_mb_monthly_consumed(int account_id, int product_id, int min_product_id, int max_product_id)
{
    //-------------------
    //get current time
    time_t current_time_tik;
    time(&current_time_tik);

    //-------------------
    //caculate start time and end time
    tm current_time_struct;
    localtime_r(&current_time_tik,&current_time_struct);

    tm start_time_struct = current_time_struct;
    start_time_struct.tm_mday = 1;
    start_time_struct.tm_hour = 0;
    start_time_struct.tm_min = 0;
    start_time_struct.tm_sec = 0;

    time_t start_time_tik = mktime(&start_time_struct);

    tm end_time_struct = current_time_struct;
    end_time_struct.tm_mday = 1;
    end_time_struct.tm_hour = 0;
    end_time_struct.tm_min = 0;
    end_time_struct.tm_sec = 0;
    end_time_struct.tm_mon = (current_time_struct.tm_mon + 1) % 12;
    if(end_time_struct.tm_mon < current_time_struct.tm_mon)
    {
        end_time_struct.tm_year++;
    }

    time_t end_time_tik = mktime(&end_time_struct);

    //-------------------
    //get monthly consumed mb according to product id interval
    if(product_id < 10000)
    {
        return 0;
    }
    else if(product_id >= 10000 && product_id < 20000)//mole
    {
        return 0;
    }
    else if(product_id >= 20000 && product_id < 30000)//seer
    {
        return 0;
    }
    else if(product_id >= 30000 && product_id < 100000)
    {
        return -1;
    }
    else
    {
        // 根据函数参数中指定的min_product_id和max_product_id查询
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(account_id,&p_db_connection);
    assert(p_db_connection != NULL);

    sprintf(g_sz_query_sql,"SELECT SUM(mb_num) FROM transaction_table WHERE account_id = %d AND transaction_time >= FROM_UNIXTIME(%d) AND transaction_time < FROM_UNIXTIME(%d) AND mb_num < 0 AND product_id >= %d AND product_id < %d",
            account_id,(int)start_time_tik,(int)end_time_tik, min_product_id, max_product_id);

    if(mysql_real_query(p_db_connection,g_sz_query_sql,strlen(g_sz_query_sql)))
    {
        ERROR_LOG("get monthly consumed mb db operation failed! account_id = %d, [SQL:%s]",account_id, g_sz_query_sql);
        return -1;
    }

    mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
    c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set);

    if(NULL == p_record_set)
    {
        return -1;
    }

    mysql_row_t data_row = mysql_fetch_row(p_record_set);
    if(data_row)
    {
        if(data_row[0])
        {
            int total_consumed_mb = atoi(data_row[0]) * -1;
            return total_consumed_mb;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//
//0 if success or error code if failed

int consume_mb(int channel_id,int account_id,int dest_account_id,int product_id,int product_count,int mb_num_to_consume,int consume_trans_id,uint64_t* p_transaction_id,uint32_t* p_mb_num_balance)
{
    //-------------------
    //param check
    if(account_id <= 0 || dest_account_id <= 0 || mb_num_to_consume <= 0 || product_id <= 0 || product_count <= 0)
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
    sprintf(g_sz_query_sql,"SELECT mb_num,is_allow_transfer,status,month_consume_limit,per_consume_limit FROM account_table WHERE account_id = %d",account_id);
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

    int mb_num_balance = atoi(data_row[0]);

    if(mb_num_balance < mb_num_to_consume)
    {
        return PACK_E_NOT_ENOUGH_MB;
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
    //check whether beyond monthly consume limit
    if(product_id >= 100000 && product_id <= 999999)
    {
        // 查找商品ID对应的范围和消费上限, 如果没找到就是没限制
        vector<consume_limit_entry_t>::iterator it;
        for (it=g_consume_limit_list.begin(); it!=g_consume_limit_list.end(); ++it)
        {
            if ((product_id >= (*it).start) && (product_id <(*it).end))
            {
                break;
            }
        }

        int monthly_consumed_mb = 0;
        int monthly_consume_limit = 0;
        if (it != g_consume_limit_list.end())
        {
            monthly_consumed_mb = get_mb_monthly_consumed(account_id, product_id, (*it).start, (*it).end);
            if (monthly_consumed_mb == -1)
            {
                return PACK_E_OPERATION_FAILED;
            }

            monthly_consume_limit = (*it).limit;
        }

        TRACE_LOG("pdt_id: %u, min: %u, max: %u, consumed: %u, limit: %u", product_id, (*it).start, (*it).end,
                monthly_consumed_mb, monthly_consume_limit);

        if (monthly_consume_limit > 0 && (monthly_consumed_mb + mb_num_to_consume > monthly_consume_limit))
        {
            return PACK_E_BEYOND_MONTH_CONSUME_LIMIT;
        }
    }

    //-------------------
    //consume mb,do db operation
    try
    {
        //-------------------
        //begin db transaction
        if(mysql_autocommit(p_db_connection,false))
        {
            throw c_my_exception_t(mysql_errno(p_db_connection),mysql_error(p_db_connection));
        }

        //-------------------
        //update mb account
        sprintf(g_sz_query_sql,"UPDATE account_table SET mb_num = mb_num - %d WHERE account_id = %d AND mb_num >= %d",mb_num_to_consume,account_id,mb_num_to_consume);
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
                //not enough mb
                ERROR_LOG("race condition lead to not enough mb!!! account_id = %d",account_id);
                throw c_my_exception_t(0,"not enough mb!");
            }
        }

        //-------------------
        //get new mb account balance
        sprintf(g_sz_query_sql,"SELECT mb_num FROM account_table WHERE account_id = %d",account_id);
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
            throw c_my_exception_t(0,"consume mb unexpected error!");
        }

        int mb_account_balance = atoi(data_row[0]);
        assert(mb_account_balance >= 0);

        //-------------------
        //write transaction log
        sprintf(g_sz_query_sql,"INSERT INTO transaction_table(account_id,dest_account_id,channel_id,pay_gate_trans_id,product_id,product_count,mb_num,mb_account_balance) \
            VALUES(%d,%d,%d,%d,%d,%d,%d,%d)",account_id,dest_account_id,channel_id,consume_trans_id,product_id,product_count,-mb_num_to_consume,mb_account_balance);

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
        //consume mb statistic
        g_statistic_mgr.consume_mb(account_id,mb_num_to_consume,channel_id,product_id,product_count);

        //-------------------
        //return transaction id and mb num balance
        if(p_transaction_id)
        {
            *p_transaction_id = transaction_id;
        }

        if(p_mb_num_balance)
        {
            *p_mb_num_balance = mb_account_balance;
        }

        return PACK_S_OK;
    }
    catch(c_my_exception_t& exception)
    {
        //-------------------
        //catch exception
        mysql_rollback(p_db_connection);
        mysql_autocommit(p_db_connection,true);

        ERROR_LOG("consume mb db operation failed! channel_id = %d,account_id = %d,mb_num = %d,product_id = %d,consume_trans_id = %d,error info = %s",
            channel_id,account_id,mb_num_to_consume,product_id,consume_trans_id,exception.get_exception_message());

        return PACK_E_OPERATION_FAILED;
    }
}

//-----------------------------------//
//-----------------------------------//
//return 0 if success or error code if failed

int get_mb_num_balance(int account_id,int* p_mb_num_balance,int* p_account_status)
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
    sprintf(g_sz_query_sql,"SELECT mb_num,status FROM account_table WHERE account_id = %d",account_id);
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

    if(p_mb_num_balance)
    {
        *p_mb_num_balance = atoi(data_row[0]);
    }

    if(p_account_status)
    {
        *p_account_status = atoi(data_row[1]);
    }

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int get_trans_info(uint64_t transaction_id,mb_proto_query_trans_ack_t* p_proto_ack)
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
    sprintf(g_sz_query_sql,"SELECT min(transaction_id) FROM transaction_table");
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
    sprintf(g_sz_query_sql,"SELECT account_id,dest_account_id,UNIX_TIMESTAMP(transaction_time),channel_id,pay_gate_trans_id,product_id,product_count,mb_num FROM transaction_table WHERE transaction_id = %u",(uint32_t)transaction_id);
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
        return PACK_E_INVALID_TRANS_ID;
    }

    //-------------------
    //return
    p_proto_ack->account_id = atoi(data_row[0]);
    p_proto_ack->dest_account_id = atoi(data_row[1]);
    p_proto_ack->transaction_time = atoi(data_row[2]);
    p_proto_ack->channel_id = atoi(data_row[3]);
    p_proto_ack->pay_gate_trans_id = atoi(data_row[4]);
    p_proto_ack->product_id = atoi(data_row[5]);
    p_proto_ack->product_count = atoi(data_row[6]);
    p_proto_ack->mb_num = atoi(data_row[7]);

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int get_channel_trans_info(uint16_t channel_id,uint32_t trans_id,mb_proto_query_channel_trans_ack_t* p_proto_ack)
{
    //-------------------
    //param check
    if(channel_id <= 0 || trans_id <= 0 || NULL == p_proto_ack)
    {
        return PACK_E_INVALID_PARAM;
    }

    //-------------------
    //get db connection
    mysql_connection_t* p_db_connection = NULL;
    g_db_connection_mgr.get_db_connection(0,&p_db_connection);
    assert(p_db_connection != NULL);

    // 由于分表扩容，需要判断交易号是否小于最小的交易号，以区分交易号不存在和交易号在历史表中的情形
    sprintf(g_sz_query_sql,"SELECT init_trans_id FROM channel_table where channel_id=%d", channel_id);
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
    if (trans_id < (uint32_t)atoi(data_row[0])) {
        return PACK_E_TRANS_SMALLER_THAN_INIT;
    }

    res_auto_ptr_query.free();

    //-------------------
    //query transaction status
    sprintf(g_sz_query_sql,"SELECT transaction_id, account_id,dest_account_id,UNIX_TIMESTAMP(transaction_time),product_id,product_count,mb_num FROM transaction_table WHERE channel_id=%u AND pay_gate_trans_id = %u",channel_id,trans_id);
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
        return PACK_E_INVALID_TRANS_ID;
    }

    //-------------------
    //return
    p_proto_ack->transaction_id = atoi(data_row[0]);
    p_proto_ack->account_id = atoi(data_row[1]);
    p_proto_ack->dest_account_id = atoi(data_row[2]);
    p_proto_ack->transaction_time = atoi(data_row[3]);
    p_proto_ack->product_id = atoi(data_row[4]);
    p_proto_ack->product_count = atoi(data_row[5]);
    p_proto_ack->mb_num = atoi(data_row[6]);

    return PACK_S_OK;
}

//-----------------------------------//
//-----------------------------------//

int query_account_history(int account_id,int start_time,int end_time,int start_row_index,int end_row_index,mb_proto_query_account_history_ack_t* p_proto_ack)
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
    sprintf(g_sz_query_sql,"SELECT COUNT(*) AS total_row_count FROM transaction_table WHERE account_id = %d AND transaction_time >= FROM_UNIXTIME(%d) AND transaction_time <= FROM_UNIXTIME(%d)",account_id,start_time,end_time);
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
    sprintf(g_sz_query_sql,"SELECT UNIX_TIMESTAMP(transaction_time),channel_id,mb_num,mb_account_balance,product_id,product_count FROM transaction_table WHERE account_id = %d AND transaction_time >= FROM_UNIXTIME(%d) AND transaction_time <= FROM_UNIXTIME(%d) ORDER BY transaction_time DESC LIMIT %d,%d",
        account_id,start_time,end_time,start_row_index,end_row_index - start_row_index);

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

    int max_data_len = sizeof(p_proto_ack->row_array) / sizeof(mb_proto_account_history_row_t);
    int data_len = 0;

    while( (data_row = mysql_fetch_row(p_second_record_set)) )
    {
        if(data_len >= max_data_len)
        {
            break;
        }

        p_proto_ack->row_array[data_len].transaction_time = atoi(data_row[0]);
        p_proto_ack->row_array[data_len].channel_id = atoi(data_row[1]);
        p_proto_ack->row_array[data_len].mb_num = atoi(data_row[2]);
        p_proto_ack->row_array[data_len].mb_account_balance = atoi(data_row[3]);
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
        const char* lpsz_config_file = "../etc/mb_account.conf";

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

        char sz_statistic_file_full_path[512];
        memset(sz_statistic_file_full_path,0,sizeof(sz_statistic_file_full_path));
        read_profile_string("statistic","file_full_path",sz_statistic_file_full_path,sizeof(sz_statistic_file_full_path),"",lpsz_config_file);

        // 读取每月消费限制
        int ret = cfp_init("../etc/consume_limit.conf");
        if (ret != 0)
        {
            ERROR_LOG("Read consume_limit.conf fail");
            return -1;
        }

        g_consume_limit_list.clear();
        cfp_goto_first_line();
        do
        {
            const char* tk = NULL;
            consume_limit_entry_t cle;

            // 第一个是起始商品ID
            tk = cfp_get_next_token(NULL, 0);
            if (tk == NULL)
            {
                continue;
            }

            if ((cle.start = strtol(tk, NULL, 0)) < 0)
            {
                ERROR_LOG("Error start product_id: %s", tk);
                return -1;
            }

            // 第二个是结束商品ID
            tk = cfp_get_next_token(NULL, 0);
            if (tk == NULL)
            {
                ERROR_LOG("NULL end product_id, check consume_limit.conf");
                return -1;
            }

            if ((cle.end = strtol(tk, NULL, 0)) <= 0)
            {
                ERROR_LOG("Error end product_id: %s", tk);
                return -1;
            }

            // 第三个是消费上限
            tk = cfp_get_next_token(NULL, 0);
            if (tk == NULL)
            {
                ERROR_LOG("NULL consume limit, check consume_limit.conf");
                return -1;
            }

            if ((cle.limit = strtol(tk, NULL, 0)) < 0)
            {
                ERROR_LOG("Error consume limit: %s", tk);
                return -1;
            }

            printf("consume limit: %d\t%d\t%d\n", cle.start, cle.end, cle.limit);
            INFO_LOG("consume limit: %d\t%d\t%d", cle.start, cle.end, cle.limit);
            g_consume_limit_list.push_back(cle);
        } while (cfp_goto_next_line() == 0);
        cfp_uninit();

        // 检查商品ID范围是否有重叠
        sort(g_consume_limit_list.begin(), g_consume_limit_list.end(), less<consume_limit_entry_t>());
        vector<consume_limit_entry_t>::iterator it;
        int last_end = 0;
        for (it=g_consume_limit_list.begin(); it!=g_consume_limit_list.end(); ++it) {
            if ((*it).start < last_end) {
                ERROR_LOG("Consumit limit product id overlapped: piror end %d, current start %d", last_end, (*it).start);
                return -1;
            }

            last_end = (*it).end;
        }

        // 读取渠道号对应的统计项
        ret = cfp_init("../etc/channel_stat.conf");
        if (ret != 0)
        {
            ERROR_LOG("Read channel_stat.conf fail");
            return -1;
        }

        g_channel_stat_map.clear();
        cfp_goto_first_line();
        do
        {
            const char* tk = NULL;
            uint32_t chnl_id = 0;
            uint32_t stat_id = 0;

            // 第一个是渠道号
            tk = cfp_get_next_token(NULL, 0);
            if (tk == NULL)
            {
                continue;
            }

            if ((chnl_id = strtol(tk, NULL, 0)) < 0)
            {
                ERROR_LOG("Error start product_id: %s", tk);
                return -1;
            }

            // 第二个是统计ID
            tk = cfp_get_next_token(NULL, 0);
            if (tk == NULL)
            {
                ERROR_LOG("NULL stat id of channel %d, check consume_limit.conf", chnl_id);
                return -1;
            }

            if ((stat_id = strtol(tk, NULL, 0)) <= 0)
            {
                ERROR_LOG("Error stat_id: %s", tk);
                return -1;
            }

            g_channel_stat_map[chnl_id] = stat_id;
            printf("added channel %u statid 0x%08X\n", chnl_id, stat_id);
            INFO_LOG("added channel %u statid 0x%08X", chnl_id, stat_id);
        } while (cfp_goto_next_line() == 0);
        cfp_uninit();

        //-------------------
        //init db connection mgr
        if(g_db_connection_mgr.init(sz_db_ip,sz_db_user,sz_db_pwd,sz_db_name,db_port))
        {
            ERROR_LOG("init db connection mgr failed! check mysql db params!");
            return -1;
        }

        //-------------------
        //init channel map
        if(init_channel_map())
        {
            ERROR_LOG("init channel map failed! check mb_channel_table!");
            g_db_connection_mgr.uninit();

            return -1;
        }

        //-------------------
        //init statistic mgr
        if(g_statistic_mgr.init(sz_statistic_file_full_path))
        {
            ERROR_LOG("init statistic mgr failed!");
            g_db_connection_mgr.uninit();
            g_channel_map.clear();

            return -1;
        }

		//-------------------
		//read IP list
		if(init_ip_list(g_allow_ips))
		{
			ERROR_LOG("read IP list config failed!");
			return -1;
		}
		else
		{
			INFO_LOG("handle_init(): %d ips are allowed.", g_allow_ips.size());
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
    CHECK_IS_GET_SERVER_PTOTO_CMD(p_buffer, length);

    //-------------------
    //get package length
    if(length < 2)
    {
        //-------------------
        //continue to recv
        return 0;
    }

    uint16_t package_length = 0;
    memcpy(&package_length,p_buffer,sizeof(package_length));

    if(package_length < sizeof(mb_proto_header_t) || package_length > 1024)
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
    *pp_send_buffer = g_sz_send_buffer;
    RETURN_IF_IS_GET_SERVER_PROTO_CMD(p_recv_buffer, recv_len, g_sz_send_buffer, *p_send_len);

    //-------------------
    //check param
    if(recv_len < (int)sizeof(mb_proto_header_t))
    {
        return -1;
    }

    //-------------------
    //get proto header
    mb_proto_header_t proto_header;
    memcpy(&proto_header,p_recv_buffer,sizeof(proto_header));

    if(proto_header.package_len != recv_len)
    {
        return -1;
    }

    //-------------------
    //deal package
    proto_header.status_code = PACK_E_UNKNOWN;
    int cmd_id = proto_header.command_id;

    if(MB_PROTO_CREATE_ACCOUNT_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_create_account_t))
        {
            return -1;
        }

        //-------------------
        //deal package
        mb_proto_create_account_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        do
        {
            //-------------------
            //try to create account
            proto_header.status_code = create_account(proto_body.account_id);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,0,proto_body.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header);
        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(MB_PROTO_ADD_MB_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_add_mb_t))
        {
            return -1;
        }

		//-------------------
		//check peer IP address
		if (check_ip(g_allow_ips, p_skinfo->remote_ip))
		{
			//not allow to access
			in_addr remote_addr;
			remote_addr.s_addr = p_skinfo->remote_ip;
			INFO_LOG("[ip:%s] forbidden", inet_ntoa(remote_addr));

			return -1;
		}

        //-------------------
        //deal add mb package
        mb_proto_add_mb_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_add_mb_ack_t proto_ack;

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
            if (proto_body.pay_gate_trans_id <= init_trans_id)
            {
                proto_header.status_code = PACK_E_TRANS_SMALLER_THAN_INIT;
                break;
            }

            //-------------------
            //add mb
            char sz_private_data_02[sizeof(proto_body.private_data_02) + 1];
            memset(sz_private_data_02,0,sizeof(sz_private_data_02));
            strncpy(sz_private_data_02,(char*)proto_body.private_data_02,sizeof(proto_body.private_data_02));

            proto_header.status_code = add_mb(proto_body.verify_header.channel_id,proto_body.account_id,proto_body.mb_num,proto_body.pay_gate_trans_id,
                proto_body.private_data_01,sz_private_data_02,&proto_ack.transaction_id);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,proto_body.verify_header.channel_id,proto_body.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(MB_POROT_CONSUME_MB_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_consume_mb_t))
        {
            return -1;
        }

        //-------------------
        //deal consume mb package
        mb_proto_consume_mb_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_consume_mb_ack_t proto_ack;

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
            if (proto_body.consume_trans_id <= init_trans_id)
            {
                proto_header.status_code = PACK_E_TRANS_SMALLER_THAN_INIT;
                break;
            }

            //-------------------
            //consume mb
            proto_header.status_code = consume_mb(proto_body.verify_header.channel_id,proto_body.account_id,proto_body.dest_account_id,proto_body.product_id,
                proto_body.product_count,proto_body.mb_num,proto_body.consume_trans_id,&proto_ack.transaction_id,&proto_ack.mb_num_balance);

        }while(0);

        //-------------------
        //print package log
        print_package_log(p_skinfo->remote_ip,cmd_id,proto_body.verify_header.channel_id,proto_body.account_id,p_recv_buffer,recv_len);

        //-------------------
        //send ack package
        proto_header.package_len = sizeof(proto_header) + sizeof(proto_ack);

        memcpy(g_sz_send_buffer,&proto_header,sizeof(proto_header));
        memcpy(g_sz_send_buffer + sizeof(proto_header),&proto_ack,sizeof(proto_ack));

        *pp_send_buffer = g_sz_send_buffer;
        *p_send_len = proto_header.package_len;

        return 0;
    }
    else if(MB_PROTO_MB_TRANSFER_CMD_ID == cmd_id)
    {
        return -1;
    }
    else if(MB_PROTO_QUERY_BALANCE_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_query_balance_t))
        {
            return -1;
        }

        //-------------------
        //deal query mb balance package
        mb_proto_query_balance_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_query_balance_ack_t proto_ack;

        do
        {
            //-------------------
            //query balance and account status
            proto_header.status_code = get_mb_num_balance(proto_body.account_id,(int*)&proto_ack.mb_num_balance,(int*)&proto_ack.account_status);

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
    else if(MB_PROTO_QUERY_TRANS_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_query_trans_t))
        {
            return -1;
        }

        //-------------------
        //deal query transaction package
        mb_proto_query_trans_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_query_trans_ack_t proto_ack;

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
    else if(MB_PROTO_QUERY_ACCOUNT_HISTORY_CMD_ID == cmd_id)
    {
        // 由于分表扩容，主服务端屏蔽此接口，相同功能由查询服务端提供
        return -1;

        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_query_account_history_t))
        {
            return -1;
        }

        //-------------------
        //deal query account history package
        mb_proto_query_account_history_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_query_account_history_ack_t proto_ack;

        do
        {
            //-------------------
            //query account history
            proto_header.status_code = query_account_history(proto_body.account_id,proto_body.start_time,proto_body.end_time,
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
    else if(MB_PROTO_QUERY_CHANNEL_TRANS_CMD_ID == cmd_id)
    {
        //-------------------
        //check size
        if(recv_len != sizeof(mb_proto_header_t) + sizeof(mb_proto_query_channel_trans_t))
        {
            return -1;
        }

        //-------------------
        //deal query transaction package
        mb_proto_query_channel_trans_t proto_body;
        memcpy(&proto_body,p_recv_buffer + sizeof(mb_proto_header_t),sizeof(proto_body));

        mb_proto_query_channel_trans_ack_t proto_ack;

        do
        {
            //-------------------
            //query trans info
            proto_header.status_code = get_channel_trans_info(proto_body.channel_id,proto_body.trans_id,&proto_ack);

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
        g_channel_map.clear();
    }
}

//-----------------------------------//
//-----------------------------------//
