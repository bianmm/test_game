#ifndef CMD_HANDLER_CPP_
#define CMD_HANDLER_CPP_
#include <map>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json/json.h>
#include <mysql/mysql.h>
#include "../include/beansys_iface.h"
#include "../include/benchapi.h"
#include "../include/functions.h"
#include "../include/i_mysql_iface.h"
#include "../include/proto.h"
#include "../include/md5.h"
#include "../include/cmd_handler.h"
#include "../include/functions.h"


extern i_mysql_iface *g_mysql_handler;
extern beansys_iface_t g_beansys;
extern beansys_iface_t g_diamonds;
char g_receipt_rsp_buf[10240] = { 0 };

int anypay_query(uint32_t user_id, const char *req_body_buf, int req_body_len,
                    char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code)
{
    *rsp_body_len = 0;
    
    if (req_body_len != sizeof(anypay_query_req_body_t))
    {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }
    
    anypay_query_req_body_t *req_body = (anypay_query_req_body_t *) req_body_buf;

    MYSQL *mysql_conn = NULL;
    mysql_conn = g_mysql_handler->get_conn();
    if (mysql_conn == NULL)
    {
        ERROR_LOG("mysql connection is null");
        *status_code = SYS_ERR;
        return -1;
    }

    MYSQL_ROW row;
    int ret = g_mysql_handler->select_first_row(&row, "SELECT order_id, channel_id, pay_channel_id, costs, product_id, ud, game_user_id FROM anypay_log_table WHERE order_id = %u",
                                   req_body->preorder_id);
    if (ret < 0)
    {
        ERROR_LOG("select info fail, error = %s", g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }
    else if (ret == 0)
    {
        // 记录不存在
        ERROR_LOG("select info return 0 row, preorder id: %u", req_body->preorder_id);
        *status_code = RECORD_NOT_FOUND;
        return -1;
    }
    else
    {
        // 获取成功
    }

    *status_code = 0;
    *rsp_body_len = sizeof(anypay_query_rsp_body_t);
    anypay_query_rsp_body_t *rsp_body =
    (anypay_query_rsp_body_t *) rsp_body_buf;
    
    rsp_body->preorder_id = atoi_safe(row[0]);
    rsp_body->channel_id = atoi_safe(row[1]);
    rsp_body->pay_channel_id = atoi_safe(row[2]);
    rsp_body->costs = atoi_safe(row[3]);
    strncpy((char*)rsp_body->product_id, row[4], sizeof(rsp_body->product_id));
    strncpy((char*)rsp_body->ud, row[5], sizeof(rsp_body->ud));
    rsp_body->user_id = atoi_safe(row[6]);
    
    return 0;
}

int anypay_preorder(uint32_t user_id, const char *req_body_buf, int req_body_len,
		char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code)
{
	*rsp_body_len = 0;
	TRACE_LOG("ipay_notify1 1");
	
    if (req_body_len != sizeof(anypay_preorder_req_body_t))
    {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify1 2");

	if (user_id == 0)
    {
		ERROR_LOG("user_id is zero!");
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify1 3");
	
    int ret = g_beansys.ping();
	TRACE_LOG("ipay_notify1 4");
	if (ret != 0)
    {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}
    ret = g_diamonds.ping();
    if (ret != 0)
    {
        ERROR_LOG("Connect to diamonds fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
    TRACE_LOG("ipay_notify1 5");
	
    anypay_preorder_req_body_t *req_body = (anypay_preorder_req_body_t *) req_body_buf;
	char ud[sizeof(req_body->ud) + 1] = { 0 };
	TRACE_LOG("ipay_notify1 6");
    
	memcpy(ud, req_body->ud, sizeof(req_body->ud));
	TRACE_LOG("ipay_notify1 7");
	trim_str(ud);
	if (strlen(ud) <= 0)
    {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify1 8");
	
    
    MYSQL *mysql_conn = NULL;
	TRACE_LOG("ipay_notify1 9");
	mysql_conn = g_mysql_handler->get_conn();
	TRACE_LOG("ipay_notify1 10");
	if (mysql_conn == NULL)
    {
		ERROR_LOG("mysql connection is null");
		*status_code = SYS_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify1 11");
	
    
    char ud_mysql[2 * sizeof(req_body->ud) + 1] = { 0 };
	mysql_real_escape_string(mysql_conn, ud_mysql, ud, strlen(ud));
    
    char product_id_mysql[2 * sizeof(req_body->product_id) + 1] = { 0 };
    mysql_real_escape_string(mysql_conn, product_id_mysql, req_body->product_id, strlen(req_body->product_id));
    
    char costs_mysql[15] = { 0 };
    sprintf(costs_mysql, "%d", req_body->costs);
    
	TRACE_LOG("ipay_notify1 12");
	ret = g_mysql_handler->execsql(
					"INSERT INTO anypay_log_table (game_user_id, ud, product_id, costs, preorder_time) VALUES (%d, \"%s\", \"%s\", \"%s\", NOW()) ON DUPLICATE KEY UPDATE game_user_id = %d, ud = \"%s\",product_id = \"%s\",  costs = \"%s\", preorder_time = NOW()",
					user_id, ud_mysql, product_id_mysql, costs_mysql,
					user_id, ud_mysql, product_id_mysql, costs_mysql);
	if (ret < 0)
    {
		ERROR_LOG("insert (%d, '%s','%s', '%s') failed, error = %s", user_id,
				ud_mysql, product_id_mysql, costs_mysql,
				g_mysql_handler->get_last_errstr());
		*status_code = DB_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify1 13");
	
    if (ret == 0)
    {
		// 记录已经存在
        ERROR_LOG("insert (%d, '%s','%s', '%s') affected 0 row", user_id,
                  ud_mysql, product_id_mysql, costs_mysql);
		*status_code = DB_ERR;
		return -1;
	}
    else
    {
		// 插入成功
	}
    TRACE_LOG("ipay_notify1 14");
	
    
    long preorder_id = mysql_insert_id(mysql_conn);
	TRACE_LOG("ipay_notify1 15");
	*status_code = 0;
	*rsp_body_len = sizeof(anypay_preorder_rsp_body_t);
	anypay_preorder_rsp_body_t *rsp_body =
			(anypay_preorder_rsp_body_t *) rsp_body_buf;
	
	rsp_body->costs = req_body->costs;
	strcpy(rsp_body->product_id,req_body->product_id);
	rsp_body->preorder_id = preorder_id;
    
	return 0;
}


int anypay_notify(int cmd_id, uint32_t userid, const char *req_body_buf, int req_body_len, char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code)
{
	*rsp_body_len = 0;
	if (req_body_len <= (int) sizeof(anypay_notify_req_body_t))
    {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify 1");
	
    int ret = g_beansys.ping();

	if (ret != 0)
    {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify 2");
	
    
    anypay_notify_req_body_t *req_body = (anypay_notify_req_body_t *) req_body_buf;
	DEBUG_LOG("transdata_len is %u", req_body->transdata_len);
	if (req_body->transdata_len >= MAX_SNDBUF_LEN)
    {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify 3");
	
    
    char transdata[MAX_SNDBUF_LEN] = { 0 };
	memcpy(transdata, req_body->data, req_body->transdata_len);
	DEBUG_LOG("transdata is %s", transdata);
    
	if (strlen(transdata) <= 0)
    {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify 4");
    
    
    
	MYSQL *mysql_conn = NULL;
	mysql_conn = g_mysql_handler->get_conn();
	if (mysql_conn == NULL) {
		ERROR_LOG("mysql connection is null while confirm apple pay");
		*status_code = SYS_ERR;
		return -1;
	}
    TRACE_LOG("ipay_notify 5");
    
	//解析字符串
    std::string preorder_id;
    std::string extern_order_id;
    std::string channel_id;
    std::string pay_channel_id;
	
    //商品相关信息
    std::string product_id;
    std::string product_name;
    std::string product_count;
    
    //支付相关信息
    std::string pay_costs;
    std::string pay_time;
    std::string pay_status;
    
    //用户相关信息
    std::string channel_user_id;
    std::string game_user_id;
    
	
	json_object * jobj = json_tokener_parse(transdata);

	enum json_type type;
	json_object_object_foreach(jobj, key, val)
	{ /*Passing through every array element*/
		type = json_object_get_type(val);
		switch (type) {
		case json_type_string:
			if (strcmp(key, "order_id") == 0) {
				extern_order_id = json_object_get_string(val);
			}
			if (strcmp(key, "channel_id") == 0) {
				channel_id = json_object_get_string(val);
			}
			if (strcmp(key, "pay_channel_id") == 0) {
				pay_channel_id = json_object_get_string(val);
			}
			if (strcmp(key, "product_id") == 0) {
				product_id = json_object_get_string(val);
			}
			if (strcmp(key, "product_name") == 0) {
				product_name = json_object_get_string(val);
			}
			if (strcmp(key, "product_count") == 0) {
				product_count = json_object_get_string(val);
			}
			if (strcmp(key, "pay_costs") == 0) {
				pay_costs = json_object_get_string(val);
			}
			if (strcmp(key, "pay_time") == 0) {
				pay_time = json_object_get_string(val);
			}
			if (strcmp(key, "pay_status") == 0) {
				pay_status = json_object_get_string(val);
			}
            if (strcmp(key, "channel_user_id") == 0) {
                channel_user_id = json_object_get_string(val);
            }
            if (strcmp(key, "game_user_id") == 0) {
                game_user_id = json_object_get_string(val);
            }
            if (strcmp(key, "preorder_id") == 0) {
                preorder_id = json_object_get_string(val);
            }

			break;
		}
	}
    TRACE_LOG("ipay_notify 6, pay_costs: %s", pay_costs.c_str());
    
    int channel_id_int = atoi_safe(channel_id.c_str());
    
	int user_id = atoi_safe(game_user_id.c_str());
	if (user_id == 0)
    {
        if ((channel_id_int == 131) || (channel_id_int == 132) || (channel_id_int == 133) ||
            (channel_id_int == 156) || (channel_id_int == 161) || (channel_id_int == 1020)) {
            // 电信爱游戏，移动咪咕和联通没有uid
        }
        else {
            *status_code = CMD_PARAM_ERR;
            return -1;
        }
	}
    
	   
	int trans_id = atoi_safe(preorder_id.c_str());
	if (trans_id <= 0)
    {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

    
    
    //检验是否支付成功 不同渠道不同
    switch (cmd_id)
    {
        case ANYPAY_NOTIFY_CMD_ID:
            if (strcmp(pay_status.c_str(), "1") != 0)
            {
                *status_code = CMD_PARAM_ERR;
                return -1;
            }
            break;
        case PP_PAY_NOTIFY_CMD_ID:
            if (strcmp(pay_status.c_str(), "1") == 0)
            {
                *status_code = 0;
                *rsp_body_len = sizeof(anypay_notify_rsp_body_t);
                anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
                rsp_body->verify_result = 0;    // 处理成功
                rsp_body->preorder_id = trans_id;
                return 0;
            }
            else if(strcmp(pay_status.c_str(), "0") != 0)
            {
                *status_code = CMD_PARAM_ERR;
                return -1;
            }
            break;
        default:
            break;
    }
	
    TRACE_LOG("ipay_notify 7");
	

    
	MYSQL_ROW row;
	ret = 0;
	// 开始事务
	TRACE_LOG("ipay_notify 8");
	ret = mysql_autocommit(mysql_conn, false);
	if (ret != 0)
    {
		ERROR_LOG("set autocommit false failed, error = %s",
				mysql_error(mysql_conn));
		*status_code = DB_ERR;
		return -1;
	}
    
	// 判断是否已经处理过
	ret = g_mysql_handler->select_first_row(&row,
					"SELECT pay_status, costs, game_user_id, product_id FROM anypay_log_table WHERE order_id = %d FOR UPDATE",
					trans_id);

	if (ret < 0)
    {
		ERROR_LOG("get status error = %s,db error",	g_mysql_handler->get_last_errstr());
		*status_code = DB_ERR;

		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0)
        {
			ERROR_LOG("set autocommit true error = %s",	mysql_error(mysql_conn));
		}

		return -1;
	}
	if (ret == 0)
    {
		// 预交易记录不存在
		ERROR_LOG("fatal error, anypay preorder trans not exists,  preorder_id is %d",	trans_id);
		*status_code = CMD_PARAM_ERR;
		*rsp_body_len = sizeof(anypay_notify_rsp_body_t);
		anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
		rsp_body->verify_result = 2;  // 预交易不存在
		rsp_body->preorder_id = trans_id;
		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0)
        {
			ERROR_LOG("set autocommit true error = %s",
					mysql_error(mysql_conn));
		}

		return 0;
	}
    TRACE_LOG("ipay_notify 9");
    
    
    
	int status = atoi_safe(row[0]);
	if (status >= RECEIPT_DEALED)
    {
		*status_code = 0;
		*rsp_body_len = sizeof(anypay_notify_rsp_body_t);
		anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
		rsp_body->verify_result = 1;  // 交易已经处理
		rsp_body->preorder_id = trans_id;
		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0)
        {
			ERROR_LOG("set autocommit true error = %s",
					mysql_error(mysql_conn));
		}
		return 0;
	}
    TRACE_LOG("ipay_notify 10");
    
    if(strcasecmp(row[1], pay_costs.c_str()) != 0)
    {
        DEBUG_LOG("costs not consistent, preorder_cost %s, anysdk_cost %s, uid is %d", row[1], pay_costs.c_str(), user_id);
    }
	
	int user_id_db = atoi_safe(row[2]);
	if (user_id_db != user_id)
    {
        user_id = user_id_db;
		DEBUG_LOG("user_id not consistent, preorder_userid: %d, anysdk_userid:%d", user_id_db, user_id);
        user_id = user_id_db;
	}
    
    char str_product_id[64] = {0};
    strcpy(str_product_id, row[3]);
    if(strcasecmp(str_product_id, product_id.c_str()) != 0)
    {
        DEBUG_LOG("product id not consistent, preorder_product_id %s, anysdk_product_id %s, uid is %d", row[3], product_id.c_str(), user_id);
        //strcpy(str_product_id, product_id.c_str());
    }
    
    //按照anysdk传过来的product id发货
	int golds_to_add = 0;
    int diamonds_to_add = 0;
    int others_goods_to_add = 0;
    int costs = 0;
    int item_id = 0;
    
    if (channel_id_int == 1002) {
        int pay_cost_int = atoi_safe(pay_costs.c_str());
        
        // 老郭，越南充值渠道特殊处理
        if (strcasecmp(product_id.c_str(), "2001") == 0) {
            // 3万
            costs = 300;
            golds_to_add = 30000;
        }
        if (strcasecmp(product_id.c_str(), "2011") == 0) {
            // 6万
            costs = 600;
            golds_to_add = 60000;
        }
        if (strcasecmp(product_id.c_str(), "2021") == 0) {
            // 15万
            costs = 1500;
            golds_to_add = 150000;
        }
        if (strcasecmp(product_id.c_str(), "2031") == 0) {
            // 35万
            costs = 3000;
            golds_to_add = 305000;
        }
        if (strcasecmp(product_id.c_str(), "2041") == 0) {
            // 80万
            costs = 6000;
            golds_to_add = 615000;
        }
        if (strcasecmp(product_id.c_str(), "2051") == 0) {
            // 250万
            costs = 15000;
            golds_to_add = 1550000;
        }
        if (strcasecmp(product_id.c_str(), "2061") == 0) {
            // 300万
            costs = 30000;
            golds_to_add = 3150000;
        }
        else if (strcasecmp(product_id.c_str(), "2002") == 0) {
            // 30钻
            costs = 300;
            diamonds_to_add = 30;
        }
        else if (strcasecmp(product_id.c_str(), "2012") == 0) {
            // 60钻
            costs = 600;
            diamonds_to_add = 60;
        }
        else if (strcasecmp(product_id.c_str(), "2022") == 0) {
            // 150钻
            costs = 1500;
            diamonds_to_add = 150;
        }
        else if (strcasecmp(product_id.c_str(), "2032") == 0) {
            // 300钻
            costs = 3000;
            diamonds_to_add = 300;
        }
        else if (strcasecmp(product_id.c_str(), "2042") == 0) {
            // 600钻
            costs = 6000;
            diamonds_to_add = 600;
        }
        else if (strcasecmp(product_id.c_str(), "2052") == 0) {
            // 1500钻
            costs = 15000;
            diamonds_to_add = 1500;
        }
        else if (strcasecmp(product_id.c_str(), "2003") == 0) {
            // 寒冰炮
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else if (strcasecmp(product_id.c_str(), "2004") == 0) {
            // 电磁炮
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 952;
        }
        else if (strcasecmp(product_id.c_str(), "2005") == 0) {
            // 镭射炮
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 953;
        }
        else {
            ERROR_LOG("unknown product_id: %s", product_id.c_str());
        }
    }
    else if(cmd_id == ANYPAY_NOTIFY_CMD_ID || cmd_id == PP_PAY_NOTIFY_CMD_ID || cmd_id == XY_PAY_NOTIFY_CMD_ID)
    {
        int pay_cost_int = atoi_safe(pay_costs.c_str()) * 100;
        
        //安卓渠道金币
        if (strcasecmp(str_product_id, "1") == 0) {
            golds_to_add = 10000;
            costs = 100;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11) || (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "2") == 0) {
            golds_to_add = 50000;
            costs = 500;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "3") == 0) {
            golds_to_add = 100000;
            costs = 1000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "4") == 0) {
            golds_to_add = 350000;
            costs = 3000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "5") == 0) {
            golds_to_add = 600000;
            costs = 5000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "6") == 0) {
            golds_to_add = 1250000;
            costs = 10000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "7") == 0) {
            golds_to_add = 2560000;
            costs = 20000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "8") == 0) {
            golds_to_add = 8300000;
            costs = 60000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        else if (strcasecmp(str_product_id, "9") == 0) {
            golds_to_add = 100;
            costs = 1;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    golds_to_add = pay_cost_int * 100;
                }
            }
        }
        //安卓渠道钻石
        else if (strcasecmp(str_product_id, "10") == 0) {
            diamonds_to_add = 50;
            costs = 500;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "11") == 0) {
            diamonds_to_add = 100;
            costs = 1000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "12") == 0) {
            diamonds_to_add = 300;
            costs = 3000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "13") == 0) {
            diamonds_to_add = 500;
            costs = 5000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "14") == 0) {
            diamonds_to_add = 1000;
            costs = 10000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "15") == 0) {
            diamonds_to_add = 2000;
            costs = 20000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        else if (strcasecmp(str_product_id, "16") == 0) {
            diamonds_to_add = 6000;
            costs = 60000;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    costs = pay_cost_int;
                    diamonds_to_add = pay_cost_int / 10;
                }
            }
        }
        //其他商品
        else if (strcasecmp(str_product_id, "101") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle item cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    
                    others_goods_to_add = 0;
                    costs = 0;
                    item_id = 0;
                }
            }
        }
        else if (strcasecmp(str_product_id, "102") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 952;
            
            if ((channel_id_int == 8) || (channel_id_int == 114) || (channel_id_int == 11)|| (channel_id_int == 122)) {
                // 当乐渠道验证costs
                if (pay_cost_int != costs) {
                    DEBUG_LOG("dangle item cost %d != expected cost %d, kill it", pay_cost_int, costs);
                    
                    others_goods_to_add = 0;
                    costs = 0;
                    item_id = 0;
                }
            }
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %s", user_id, str_product_id);
        }
    }
    else if(cmd_id == BAIDU_SIGNE_NOTIFY_CMD_ID)
    {
        //安卓渠道金币
        if (strcasecmp(str_product_id, "10190") == 0) {
            golds_to_add = 50000;
            costs = 500;
        }
        else if (strcasecmp(str_product_id, "10191") == 0) {
            golds_to_add = 100000;
            costs = 1000;
        }
        else if (strcasecmp(str_product_id, "10192") == 0) {
            golds_to_add = 100;//测试商品
            costs = 1;
        }
        else if (strcasecmp(str_product_id, "10193") == 0) {
            golds_to_add = 350000;
            costs = 3000;
        }
        else if (strcasecmp(str_product_id, "10194") == 0) {
            golds_to_add = 600000;
            costs = 5000;
        }
        else if (strcasecmp(str_product_id, "10195") == 0) {
            golds_to_add = 1250000;
            costs = 10000;
        }
        else if (strcasecmp(str_product_id, "10196") == 0) {
            golds_to_add = 2560000;
            costs = 20000;
        }
        else if (strcasecmp(str_product_id, "10197") == 0) {
            golds_to_add = 8300000;
            costs = 60000;
        }
        
        //安卓渠道钻石
        else if (strcasecmp(str_product_id, "10198") == 0) {
            diamonds_to_add = 50;
            costs = 500;
        }
        else if (strcasecmp(str_product_id, "10199") == 0) {
            diamonds_to_add = 100;
            costs = 1000;
        }
        else if (strcasecmp(str_product_id, "10200") == 0) {
            diamonds_to_add = 300;
            costs = 3000;
        }
        else if (strcasecmp(str_product_id, "10201") == 0) {
            diamonds_to_add = 500;
            costs = 5000;
        }
        else if (strcasecmp(str_product_id, "10202") == 0) {
            diamonds_to_add = 1000;
            costs = 10000;
        }
        else if (strcasecmp(str_product_id, "10203") == 0) {
            diamonds_to_add = 2000;
            costs = 20000;
        }
        else if (strcasecmp(str_product_id, "10204") == 0) {
            diamonds_to_add = 6000;
            costs = 60000;
        }
        //其他商品
        else if (strcasecmp(str_product_id, "10234") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %s", user_id, str_product_id);
        }
        
    }
    else if(cmd_id == CHONGCHONG_PAY_NOTIFY_CMD_ID)
    {
        //安卓渠道金币
        if(strcasecmp(str_product_id, "101108") == 0){
            golds_to_add = 10000;
            costs = 100;
        }
        else if (strcasecmp(str_product_id, "101109") == 0) {
            golds_to_add = 50000;
            costs = 500;
        }
        else if (strcasecmp(str_product_id, "101110") == 0) {
            golds_to_add = 100000;
            costs = 1000;
        }
        else if (strcasecmp(str_product_id, "101145") == 0) {
            golds_to_add = 1;//测试商品
            costs = 1;
        }
        else if (strcasecmp(str_product_id, "101111") == 0) {
            golds_to_add = 350000;
            costs = 3000;
        }
        else if (strcasecmp(str_product_id, "101112") == 0) {
            golds_to_add = 600000;
            costs = 5000;
        }
        else if (strcasecmp(str_product_id, "101113") == 0) {
            golds_to_add = 1250000;
            costs = 10000;
        }
        else if (strcasecmp(str_product_id, "101114") == 0) {
            golds_to_add = 2560000;
            costs = 20000;
        }
        else if (strcasecmp(str_product_id, "101115") == 0) {
            golds_to_add = 8300000;
            costs = 60000;
        }
        
        //安卓渠道钻石
        else if (strcasecmp(str_product_id, "101116") == 0) {
            diamonds_to_add = 50;
            costs = 500;
        }
        else if (strcasecmp(str_product_id, "101117") == 0) {
            diamonds_to_add = 100;
            costs = 1000;
        }
        else if (strcasecmp(str_product_id, "101118") == 0) {
            diamonds_to_add = 300;
            costs = 3000;
        }
        else if (strcasecmp(str_product_id, "101119") == 0) {
            diamonds_to_add = 500;
            costs = 5000;
        }
        else if (strcasecmp(str_product_id, "101120") == 0) {
            diamonds_to_add = 1000;
            costs = 10000;
        }
        else if (strcasecmp(str_product_id, "101121") == 0) {
            diamonds_to_add = 2000;
            costs = 20000;
        }
        else if (strcasecmp(str_product_id, "101122") == 0) {
            diamonds_to_add = 6000;
            costs = 60000;
        }
        //其他商品
        else if (strcasecmp(str_product_id, "101123") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %s", user_id, str_product_id);
        }

    }
    /*
     else if(cmd_id == PP_PAY_NOTIFY_CMD_ID)
     {
     //苹果渠道钻石
     if (strcasecmp(str_product_id, "10") == 0) {
     diamonds_to_add = 60;
     costs = 600;
     }
     else if (strcasecmp(str_product_id, "11") == 0) {
     diamonds_to_add = 120;
     costs = 1200;
     }
     else if (strcasecmp(str_product_id, "12") == 0) {
     diamonds_to_add = 300;
     costs = 3000;
     }
     else if (strcasecmp(str_product_id, "13") == 0) {
     diamonds_to_add = 500;
     costs = 5000;
     }
     else if (strcasecmp(str_product_id, "14") == 0) {
     diamonds_to_add = 1280;
     costs = 12800;
     }
     else if (strcasecmp(str_product_id, "15") == 0) {
     diamonds_to_add = 2580;
     costs = 25800;
     }
     else if (strcasecmp(str_product_id, "16") == 0) {
     diamonds_to_add = 6480;
     costs = 64800;
     }
     }
     */
    else
    {
        // 未知的命令id
        ERROR_LOG("Unknown cmd id: %s",  cmd_id);
    }

    TRACE_LOG("ipay_notify 11");
    
    //先写入订单信息，此时验证已经通过，订单状态是RECEIPT_VERIFIED
    int count = atoi_safe(product_count.c_str());
    int ext_trans_id = atoi_safe(extern_order_id.c_str());
    ret = g_mysql_handler->execsql(
                                   "UPDATE anypay_log_table SET extern_order_id = \"%s\", channel_id = \"%s\", pay_channel_id = \"%s\", product_name = \"%s\", product_count = %d, pay_time = \"%s\", pay_status = %d, deal_time = NOW(), channel_user_id = \"%s\" WHERE order_id = %d",
                                    extern_order_id.c_str(), channel_id.c_str(), pay_channel_id.c_str(), product_name.c_str(), count, pay_time.c_str(),  RECEIPT_VERIFIED, channel_user_id.c_str(), trans_id);
    if (ret < 0)
    {
        ERROR_LOG("update ipay log failed, error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        
        ret = mysql_rollback(mysql_conn);
        if (ret != 0) {
            ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
        }
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        return -1;
    }
    TRACE_LOG("ipay_notify 12");
    
    // 发货
    int32_t result = -1;
    uint64_t beansys_trans_id = 0;
    
	if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
    {
        if(golds_to_add > 0)
        {
            if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, trans_id, channel_id_int, &result,&beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0)
                {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0)
                {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        else if(diamonds_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, diamonds_to_add, costs, item_id, trans_id, channel_id_int, &result,	&beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0)
                {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0)
                {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        else if(others_goods_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, others_goods_to_add, costs, item_id, trans_id, channel_id_int, &result, &beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0) {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0) {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        TRACE_LOG("ipay_notify 13");
        
        //发货成功，则将状态更新为RECEIPT_DEALED
        ret = g_mysql_handler->execsql("UPDATE anypay_log_table SET beansys_trans_id = %d, deal_time = NOW(), pay_status = %d WHERE order_id = %d;", beansys_trans_id, RECEIPT_DEALED, trans_id);
        if (ret < 0) {
            ERROR_LOG("update apple pay log failed, error = %s",
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            
            ret = mysql_commit(mysql_conn);
            if (ret != 0) {
                ERROR_LOG("commit error = %s",
                          mysql_error(mysql_conn));
            }
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
        TRACE_LOG("ipay_notify 14");
        
		if (result == BEANSYS_E_TRANS_ALREADY_DEALED)
        {
			DEBUG_LOG(
					"Add golden bean already dealed, user_id: %d, trans_id: %d",
					user_id, trans_id);
			*status_code = 0;
			*rsp_body_len = sizeof(anypay_notify_rsp_body_t);
			anypay_notify_rsp_body_t *ack_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
			ack_body->verify_result = 1;  // 交易已处理
			ack_body->preorder_id = trans_id;

			ret = mysql_commit(mysql_conn);
			if (ret != 0) {
				ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
			}

			ret = mysql_autocommit(mysql_conn, true);
			if (ret != 0) {
				ERROR_LOG("set autocommit true error = %s",
						mysql_error(mysql_conn));
			}
            
			return 0;
		}
        else if (result != BEANSYS_S_OK)
        {
			DEBUG_LOG("Add golden bean fail, result: %d, %d", user_id, result);
			*status_code = SYS_ERR;

			ret = mysql_rollback(mysql_conn);
			if (ret != 0) {
				ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
			}

			ret = mysql_autocommit(mysql_conn, true);
			if (ret != 0) {
				ERROR_LOG("set autocommit true error = %s",
						mysql_error(mysql_conn));
			}

			return -1;
		}
        TRACE_LOG("ipay_notify 15");
	}
    else
    {
		TRACE_LOG("ipay_notify 16");
        ERROR_LOG("not add seer bean, golds_to_add is %d, costs is %d", golds_to_add, costs);
        
        //不加金币也要将状态更新为RECEIPT_DEALED
        ret = g_mysql_handler->execsql("UPDATE anypay_log_table SET beansys_trans_id = 0, deal_time = NOW(), pay_status = %d WHERE order_id = %d;", RECEIPT_DEALED, trans_id);
        if (ret < 0) {
            ERROR_LOG("update apple pay log failed, error = %s",
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            
            ret = mysql_commit(mysql_conn);
            if (ret != 0) {
                ERROR_LOG("commit error = %s",
                          mysql_error(mysql_conn));
            }
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
	}
    
    
	ret = mysql_commit(mysql_conn);
	if (ret != 0)
    {
		ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
	}
	ret = mysql_autocommit(mysql_conn, true);
	
	if (ret != 0)
    {
		ERROR_LOG("set autocommit true error = %s", mysql_error(mysql_conn));
	}
	*status_code = 0;
	*rsp_body_len = sizeof(anypay_notify_rsp_body_t);
	anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
	rsp_body->verify_result = 0;  // 处理成功
	rsp_body->preorder_id = trans_id;
    TRACE_LOG("ipay_notify 17");
	return 0;
}

int anypay_notify_client(int cmd_id, uint32_t userid, const char *req_body_buf, int req_body_len, char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code)
{
    *rsp_body_len = 0;
    if (req_body_len <= (int) sizeof(anypay_notify_client_req_body_t))
    {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }
    TRACE_LOG("ipay_notify 1");
    
    int ret = g_beansys.ping();
    
    if (ret != 0)
    {
        ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
    TRACE_LOG("ipay_notify 2");
    
    
    anypay_notify_client_req_body_t *req_body = (anypay_notify_client_req_body_t *) req_body_buf;
    DEBUG_LOG("transdata_len is %u, channel_id: %d", req_body->transdata_len, req_body->channel_id);
    if (req_body->transdata_len >= MAX_SNDBUF_LEN)
    {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    TRACE_LOG("ipay_notify 3");
    
    
    char transdata[MAX_SNDBUF_LEN] = { 0 };
    memcpy(transdata, req_body->data, req_body->transdata_len);
    DEBUG_LOG("transdata is %s", transdata);
    
    if (strlen(transdata) <= 0)
    {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    TRACE_LOG("ipay_notify 4");
    
    
    
    MYSQL *mysql_conn = NULL;
    mysql_conn = g_mysql_handler->get_conn();
    if (mysql_conn == NULL) {
        ERROR_LOG("mysql connection is null while confirm apple pay");
        *status_code = SYS_ERR;
        return -1;
    }
    
    DEBUG_LOG("ipay_notify, channel_id: %d, preorder_id: %d, trans_data: %s", req_body->channel_id, req_body->preorder_id, transdata);
    
    //商品相关信息
    std::string product_id;
    std::string product_count;
    std::string pay_costs;
    std::string token;
    
    
    json_object * jobj = json_tokener_parse(transdata);
    if (jobj == NULL) {
        ERROR_LOG("invalid trans data: %s", transdata);
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    enum json_type type;
    json_object_object_foreach(jobj, key, val)
    { /*Passing through every array element*/
        type = json_object_get_type(val);
        switch (type) {
            case json_type_string:
                if (strcmp(key, "product_id") == 0) {
                    product_id = json_object_get_string(val);
                }
                if (strcmp(key, "product_count") == 0) {
                    product_count = json_object_get_string(val);
                }
                if (strcmp(key, "pay_costs") == 0) {
                    pay_costs = json_object_get_string(val);
                }
                if (strcmp(key, "token") == 0) {
                    token = json_object_get_string(val);
                }
                
                break;
        }
    }
    TRACE_LOG("ipay_notify 6");
    
    char orig_str[4096] = {0};
    sprintf(orig_str, "%s,%s,%s,1325476", product_id.c_str(), product_count.c_str(), pay_costs.c_str());
    char* md5_str = (char*) md5_sum_32((uint8_t*)orig_str, strlen(orig_str));
    if (strcasecmp(md5_str, token.c_str()) != 0) {
        ERROR_LOG("%d: server: %s != client: %s", userid, md5_str, token.c_str());
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    
    int user_id = userid;
    if (user_id == 0)
    {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
	   
    int trans_id = req_body->preorder_id;
    if (trans_id <= 0)
    {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    
    MYSQL_ROW row;
    ret = 0;
    // 开始事务
    TRACE_LOG("ipay_notify 8");
    ret = mysql_autocommit(mysql_conn, false);
    if (ret != 0)
    {
        ERROR_LOG("set autocommit false failed, error = %s",
                  mysql_error(mysql_conn));
        *status_code = DB_ERR;
        return -1;
    }
    
    // 判断是否已经处理过
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT pay_status, costs, game_user_id, product_id FROM anypay_log_table WHERE order_id = %d FOR UPDATE",
                                            trans_id);
    
    if (ret < 0)
    {
        ERROR_LOG("get status error = %s,db error",	g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0)
        {
            ERROR_LOG("set autocommit true error = %s",	mysql_error(mysql_conn));
        }
        
        return -1;
    }
    if (ret == 0)
    {
        // 预交易记录不存在
        ERROR_LOG("fatal error, anypay preorder trans not exists,  preorder_id is %d",	trans_id);
        *status_code = CMD_PARAM_ERR;
        *rsp_body_len = sizeof(anypay_notify_rsp_body_t);
        anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
        rsp_body->verify_result = 2;  // 预交易不存在
        rsp_body->preorder_id = trans_id;
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0)
        {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        
        return 0;
    }
    TRACE_LOG("ipay_notify 9");
    
    
    
    int status = atoi_safe(row[0]);
    if (status >= RECEIPT_DEALED)
    {
        *status_code = 0;
        *rsp_body_len = sizeof(anypay_notify_rsp_body_t);
        anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
        rsp_body->verify_result = 1;  // 交易已经处理
        rsp_body->preorder_id = trans_id;
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0)
        {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        return 0;
    }
    TRACE_LOG("ipay_notify 10");
    
    if(strcasecmp(row[1], pay_costs.c_str()) != 0)
    {
        DEBUG_LOG("costs not consistent, preorder_cost %s, anysdk_cost %s, uid is %d", row[1], pay_costs.c_str(), user_id);
    }
    
    int user_id_db = atoi_safe(row[2]);
    if (user_id_db != user_id)
    {
        user_id = user_id_db;
        DEBUG_LOG("user_id not consistent, preorder_userid: %d, anysdk_userid:%d", user_id_db, user_id);
        user_id = user_id_db;
    }
    
    char str_product_id[64] = {0};
    strcpy(str_product_id, row[3]);
    if(strcasecmp(str_product_id, product_id.c_str()) != 0)
    {
        DEBUG_LOG("product id not consistent, preorder_product_id %s, anysdk_product_id %s, uid is %d", row[3], product_id.c_str(), user_id);
        //strcpy(str_product_id, product_id.c_str());
    }
    
    //按照anysdk传过来的product id发货
    int golds_to_add = 0;
    int diamonds_to_add = 0;
    int others_goods_to_add = 0;
    int costs = 0;
    int item_id = 0;
    
    //安卓渠道金币
    if (strcasecmp(str_product_id, "1") == 0) {
        golds_to_add = 10000;
        costs = 100;
    }
    else if (strcasecmp(str_product_id, "2") == 0) {
        golds_to_add = 50000;
        costs = 500;
    }
    else if (strcasecmp(str_product_id, "3") == 0) {
        golds_to_add = 100000;
        costs = 1000;
    }
    else if (strcasecmp(str_product_id, "4") == 0) {
        golds_to_add = 350000;
        costs = 3000;
    }
    else if (strcasecmp(str_product_id, "5") == 0) {
        golds_to_add = 600000;
        costs = 5000;
    }
    else if (strcasecmp(str_product_id, "6") == 0) {
        golds_to_add = 1250000;
        costs = 10000;
    }
    else if (strcasecmp(str_product_id, "7") == 0) {
        golds_to_add = 2560000;
        costs = 20000;
    }
    else if (strcasecmp(str_product_id, "8") == 0) {
        golds_to_add = 8300000;
        costs = 60000;
    }
    else if (strcasecmp(str_product_id, "9") == 0) {
        golds_to_add = 100;
        costs = 1;
    }
    //安卓渠道钻石
    else if (strcasecmp(str_product_id, "10") == 0) {
        diamonds_to_add = 50;
        costs = 500;
    }
    else if (strcasecmp(str_product_id, "11") == 0) {
        diamonds_to_add = 100;
        costs = 1000;
    }
    else if (strcasecmp(str_product_id, "12") == 0) {
        diamonds_to_add = 300;
        costs = 3000;
    }
    else if (strcasecmp(str_product_id, "13") == 0) {
        diamonds_to_add = 500;
        costs = 5000;
    }
    else if (strcasecmp(str_product_id, "14") == 0) {
        diamonds_to_add = 1000;
        costs = 10000;
    }
    else if (strcasecmp(str_product_id, "15") == 0) {
        diamonds_to_add = 2000;
        costs = 20000;
    }
    else if (strcasecmp(str_product_id, "16") == 0) {
        diamonds_to_add = 6000;
        costs = 60000;
    }
    //其他商品
    else if (strcasecmp(str_product_id, "101") == 0) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 951;
    }
    else if (strcasecmp(str_product_id, "102") == 0) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 952;
    }
    else {
        // 未知的商品id
        ERROR_LOG("Unknown pdt id: %d, %s", user_id, str_product_id);
    }
    
    
    TRACE_LOG("ipay_notify 11");
    
    //先写入订单信息，此时验证已经通过，订单状态是RECEIPT_VERIFIED
    int count = atoi_safe(product_count.c_str());
    int ext_trans_id = 0;
    ret = g_mysql_handler->execsql(
                                   "UPDATE anypay_log_table SET product_count = %d, pay_status = %d, deal_time = NOW() WHERE order_id = %d", count,  RECEIPT_VERIFIED, trans_id);
    if (ret < 0)
    {
        ERROR_LOG("update ipay log failed, error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        
        ret = mysql_rollback(mysql_conn);
        if (ret != 0) {
            ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
        }
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        return -1;
    }
    TRACE_LOG("ipay_notify 12");
    
    // 计算channel_id的整型值
    int channel_id_int = req_body->channel_id;
    
    // 发货
    int32_t result = -1;
    uint64_t beansys_trans_id = 0;
    
    if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
    {
        if(golds_to_add > 0)
        {
            if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, trans_id, channel_id_int, &result,&beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0)
                {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0)
                {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        else if(diamonds_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, diamonds_to_add, costs, item_id, trans_id, channel_id_int, &result,	&beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0)
                {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0)
                {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        else if(others_goods_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, others_goods_to_add, costs, item_id, trans_id, channel_id_int, &result, &beansys_trans_id) != 0)
            {
                ERROR_LOG("Add seer bean for user %u fail", user_id);
                *status_code = SYS_ERR;
                
                ret = mysql_rollback(mysql_conn);
                if (ret != 0) {
                    ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
                }
                
                ret = mysql_autocommit(mysql_conn, true);
                if (ret != 0) {
                    ERROR_LOG("set autocommit true error = %s",
                              mysql_error(mysql_conn));
                }
                
                return -1;
            }
        }
        TRACE_LOG("ipay_notify 13");
        
        //发货成功，则将状态更新为RECEIPT_DEALED
        ret = g_mysql_handler->execsql("UPDATE anypay_log_table SET beansys_trans_id = %d, deal_time = NOW(), pay_status = %d WHERE order_id = %d;", beansys_trans_id, RECEIPT_DEALED, trans_id);
        if (ret < 0) {
            ERROR_LOG("update apple pay log failed, error = %s",
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            
            ret = mysql_commit(mysql_conn);
            if (ret != 0) {
                ERROR_LOG("commit error = %s",
                          mysql_error(mysql_conn));
            }
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
        TRACE_LOG("ipay_notify 14");
        
        if (result == BEANSYS_E_TRANS_ALREADY_DEALED)
        {
            DEBUG_LOG(
                      "Add golden bean already dealed, user_id: %d, trans_id: %d",
                      user_id, trans_id);
            *status_code = 0;
            *rsp_body_len = sizeof(anypay_notify_rsp_body_t);
            anypay_notify_rsp_body_t *ack_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
            ack_body->verify_result = 1;  // 交易已处理
            ack_body->preorder_id = trans_id;
            
            ret = mysql_commit(mysql_conn);
            if (ret != 0) {
                ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
            }
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return 0;
        }
        else if (result != BEANSYS_S_OK)
        {
            DEBUG_LOG("Add golden bean fail, result: %d, %d", user_id, result);
            *status_code = SYS_ERR;
            
            ret = mysql_rollback(mysql_conn);
            if (ret != 0) {
                ERROR_LOG("rollback error = %s", mysql_error(mysql_conn));
            }
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
        TRACE_LOG("ipay_notify 15");
    }
    else
    {
        TRACE_LOG("ipay_notify 16");
        ERROR_LOG("not add seer bean, golds_to_add is %d, costs is %d", golds_to_add, costs);
    }
    
    
    ret = mysql_commit(mysql_conn);
    if (ret != 0)
    {
        ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
    }
    ret = mysql_autocommit(mysql_conn, true);
    
    if (ret != 0)
    {
        ERROR_LOG("set autocommit true error = %s", mysql_error(mysql_conn));
    }
    *status_code = 0;
    *rsp_body_len = sizeof(anypay_notify_rsp_body_t);
    anypay_notify_rsp_body_t *rsp_body = (anypay_notify_rsp_body_t *) rsp_body_buf;
    rsp_body->verify_result = 0;  // 处理成功
    rsp_body->preorder_id = trans_id;
    TRACE_LOG("ipay_notify 17");
    return 0;
}

#endif /* CMD_HANDLER_CPP_ */

