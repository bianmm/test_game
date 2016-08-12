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
extern i_mysql_iface *g_mysql_handler;
extern beansys_iface_t g_beansys;
extern beansys_iface_t g_diamonds;
char g_receipt_rsp_buf[10240] = { 0 };

int ipay_preorder(uint32_t user_id, const char *req_body_buf, int req_body_len,
		char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code) {
	*rsp_body_len = 0;
	TRACE_LOG("ipay_notify1 1");
	if (req_body_len != sizeof(ipay_preorder_req_body_t)) {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 2");

	if (user_id == 0) {
		ERROR_LOG("user_id is zero!");
		*status_code = CMD_PARAM_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 3");
	int ret = g_beansys.ping();
	TRACE_LOG("ipay_notify1 4");
	if (ret != 0) {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 5");
    ret = g_diamonds.ping();
    if (ret != 0) {
        ERROR_LOG("Connect to dianmods fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
	ipay_preorder_req_body_t *req_body =
			(ipay_preorder_req_body_t *) req_body_buf;
	char ud[sizeof(req_body->ud) + 1] = { 0 };
	TRACE_LOG("ipay_notify1 6");
	memcpy(ud, req_body->ud, sizeof(req_body->ud));
	TRACE_LOG("ipay_notify1 7");
	trim_str(ud);
	if (strlen(ud) <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 8");
	MYSQL *mysql_conn = NULL;
	TRACE_LOG("ipay_notify1 9");
	mysql_conn = g_mysql_handler->get_conn();
	TRACE_LOG("ipay_notify1 10");
	if (mysql_conn == NULL) {
		ERROR_LOG("mysql connection is null");
		*status_code = SYS_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 11");
	char ud_mysql[2 * sizeof(req_body->ud) + 1] = { 0 };
	mysql_real_escape_string(mysql_conn, ud_mysql, ud, strlen(ud));
	TRACE_LOG("ipay_notify1 12");
	ret =
			g_mysql_handler->execsql(
					"INSERT INTO pay_360_log_table (uid, ud,store_id,price) VALUES (%d, '%s', %d,%d) ON DUPLICATE KEY UPDATE uid = %d, ud = '%s',store_id = %d,  price = %d",
					user_id, ud_mysql, req_body->storeid, req_body->price,
					user_id, ud_mysql, req_body->storeid, req_body->price);
	if (ret < 0) {
		ERROR_LOG("insert (%d, '%s','%s', %d,%d) failed, error = %s", user_id,
				ud_mysql, req_body->storeid, req_body->price,
				g_mysql_handler->get_last_errstr());
		*status_code = DB_ERR;
		return -1;
	}TRACE_LOG("ipay_notify1 13");
	if (ret == 0) {
		// 记录已经存在
		ERROR_LOG("insert (%d, '%s','%s', %d,%d) affected 0 row", user_id,
				ud_mysql, req_body->storeid, req_body->price);
		*status_code = DB_ERR;
		return -1;
	} else {
		// 插入成功
	}TRACE_LOG("ipay_notify1 14");
	long trans_id = mysql_insert_id(mysql_conn);
	TRACE_LOG("ipay_notify1 15");
	*status_code = 0;
	*rsp_body_len = sizeof(ipay_preorder_rsp_body_t);
	ipay_preorder_rsp_body_t *rsp_body =
			(ipay_preorder_rsp_body_t *) rsp_body_buf;
	rsp_body->zoneid = req_body->zoneid;
	rsp_body->price = req_body->price;
	rsp_body->storeid = req_body->storeid;
	rsp_body->trans_id = trans_id;
	return 0;
}

std::string APP_KEY = "856e7806ed1d29a880caad887b9dd5a5";
std::string APP_SECRET = "935d86f4c0964aadc439d0baedaec962";

int ipay_notify(uint32_t userid, const char *req_body_buf, int req_body_len,
		char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code) {
	DEBUG_LOG("ipay_notify");
	*rsp_body_len = 0;
	if (req_body_len <= (int) sizeof(ipay_notify_req_body_t)) {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 1");
	int ret = g_beansys.ping();
	TRACE_LOG("ipay_notify 2");
	if (ret != 0) {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 3");
    ret = g_diamonds.ping();
    if (ret != 0) {
        ERROR_LOG("Connect to dianmods fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
    
	ipay_notify_req_body_t *req_body = (ipay_notify_req_body_t *) req_body_buf;
	TRACE_LOG("ipay_notify 4");
	DEBUG_LOG("transdata_len is %u", req_body->transdata_len);TRACE_LOG("ipay_notify 5");
	if (req_body->transdata_len >= MAX_SNDBUF_LEN) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 6");
	char transdata[MAX_SNDBUF_LEN] = { 0 };
	TRACE_LOG("ipay_notify 7");
	memcpy(transdata, req_body->data, req_body->transdata_len);
	TRACE_LOG("ipay_notify 8");
	DEBUG_LOG("transdata_len is %u", req_body->transdata_len);TRACE_LOG("ipay_notify 9");
	DEBUG_LOG("transdata is %s", transdata);TRACE_LOG("ipay_notify 10");
	if (strlen(transdata) <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 11");
	MYSQL *mysql_conn = NULL;
	mysql_conn = g_mysql_handler->get_conn();
	TRACE_LOG("ipay_notify 12");
	if (mysql_conn == NULL) {
		ERROR_LOG("mysql connection is null while confirm apple pay");
		*status_code = SYS_ERR;
		return -1;
	}
	//解析字符串
	std::string app_key;
	std::string product_id;
	std::string app_uid;
	std::string gateway_flag;
	std::string app_order_id;
	std::string sign;
	std::string sign_type;
	std::string amount_str;
	std::string user_id_360;
	std::string order_id;
	TRACE_LOG("ipay_notify 13");
	json_object * jobj = json_tokener_parse(transdata);
    if(jobj == NULL)
    {
        ERROR_LOG("json_tokener_parse return null");
        *status_code = JSON_DATA_ERR;
        return -1;
    }
	TRACE_LOG("ipay_notify 14");
	enum json_type type;
	json_object_object_foreach(jobj, key, val)
	{ /*Passing through every array element*/
		type = json_object_get_type(val);
		switch (type) {
		case json_type_string:
			if (strcmp(key, "sign") == 0) {
				sign = json_object_get_string(val);
			}
			if (strcmp(key, "amount") == 0) {
				amount_str = json_object_get_string(val);
			}
			if (strcmp(key, "app_key") == 0) {
				app_key = json_object_get_string(val);
			}
			if (strcmp(key, "product_id") == 0) {
				product_id = json_object_get_string(val);
			}
			if (strcmp(key, "app_uid") == 0) {
				app_uid = json_object_get_string(val);
			}
			if (strcmp(key, "gateway_flag") == 0) {
				gateway_flag = json_object_get_string(val);
			}
			if (strcmp(key, "app_order_id") == 0) {
				app_order_id = json_object_get_string(val);
			}
			if (strcmp(key, "sign_type") == 0) {
				sign_type = json_object_get_string(val);
			}
			if (strcmp(key, "user_id") == 0) {
				user_id_360 = json_object_get_string(val);
			}
			if (strcmp(key, "order_id") == 0) {
				order_id = json_object_get_string(val);
			}
			break;
		}
	}TRACE_LOG("ipay_notify 15");
	int user_id = atoi_safe(app_uid.c_str());
	if (user_id == 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 16");
	DEBUG_LOG("trans_id is %s", app_order_id.c_str());
	int trans_id = atoi_safe(app_order_id.c_str());
	TRACE_LOG("ipay_notify 17");
	if (trans_id <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	if (strcmp(gateway_flag.c_str(), "success") != 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	if (strcmp(app_key.c_str(), APP_KEY.c_str()) != 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	std::string key_test_string[] = { "amount", "app_key", "app_order_id",
			"app_uid", "gateway_flag", "order_id", "product_id", "sign_type",
			"user_id" };
//	sort(key_test_string, key_test_string + key_test_string->length());
	TRACE_LOG("ipay_notify 18");
	std::string sing_data = "";
	for (int var = 0; var < 9; ++var) {
		std::string key_str = key_test_string[var];
		char * key_str_char = (char *) key_str.c_str();
		if (strcmp(key_str_char, "app_key") == 0) {
			sing_data = sing_data + app_key + "#";
		}
		if (strcmp(key_str_char, "amount") == 0) {
			sing_data = sing_data + amount_str + "#";
		}
		if (strcmp(key_str_char, "product_id") == 0) {
			sing_data = sing_data + product_id + "#";
		}
		if (strcmp(key_str_char, "app_uid") == 0) {
			sing_data = sing_data + app_uid + "#";
		}
		if (strcmp(key_str_char, "order_id") == 0) {
			sing_data = sing_data + order_id + "#";
		}
		if (strcmp(key_str_char, "sign_type") == 0) {
			sing_data = sing_data + sign_type + "#";
		}
		if (strcmp(key_str_char, "gateway_flag") == 0) {
			sing_data = sing_data + gateway_flag + "#";
		}
		if (strcmp(key_str_char, "user_id") == 0) {
			sing_data = sing_data + user_id_360 + "#";
		}
		if (strcmp(key_str_char, "app_order_id") == 0) {
			sing_data = sing_data + app_order_id + "#";
		}
	}
	sing_data += APP_SECRET;
	TRACE_LOG("ipay_notify 20");
	MD5 md5(sing_data);
	TRACE_LOG("ipay_notify 21 , sign is %d",sign.length());TRACE_LOG("ipay_notify 21 , md5.md5().c_str() is %d",md5.md5().length());
	//验签
	if (strcmp(md5.md5().c_str(), sign.c_str()) != 0) {
		// 验签失败
		ERROR_LOG("ipay sign verify fail ,gateway_flag is %s,app_key is %s",
				gateway_flag.c_str(), app_key.c_str());
		*status_code = 0;
		*rsp_body_len = sizeof(ipay_notify_rsp_body_t);
		ipay_notify_rsp_body_t *rsp_body =
				(ipay_notify_rsp_body_t *) rsp_body_buf;
		rsp_body->verify_result = 3;  // 验签失败或者交易未成功或者非本游戏的充值返回
		rsp_body->trans_id = 0;
		return 0;
	}
	int amount = atoi(amount_str.c_str());
	int pay_log_id = trans_id;
	MYSQL_ROW row;
	ret = 0;
	// 开始事务
	TRACE_LOG("ipay_notify 22");
	ret = mysql_autocommit(mysql_conn, false);
	if (ret != 0) {
		ERROR_LOG("set autocommit false failed, error = %s",
				mysql_error(mysql_conn));
		*status_code = DB_ERR;
		return -1;
	}TRACE_LOG("ipay_notify 23");
	// 判断是否已经处理过
	ret =
			g_mysql_handler->select_first_row(&row,
					"SELECT id,status, price,store_id,uid FROM pay_360_log_table WHERE id = %d FOR UPDATE",
					trans_id);
	TRACE_LOG("ipay_notify 24");
	if (ret < 0) {
		ERROR_LOG("get status error = %s,db error",
				g_mysql_handler->get_last_errstr());
		*status_code = DB_ERR;

		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0) {
			ERROR_LOG("set autocommit true error = %s",
					mysql_error(mysql_conn));
		}

		return -1;
	}TRACE_LOG("ipay_notify 25");
	if (ret <= 0) {
		// 预交易记录不存在
		ERROR_LOG("fatal error, 360pay preorder trans not exists,  %d",
				trans_id);
		*status_code = 0;
		*rsp_body_len = sizeof(ipay_notify_rsp_body_t);
		ipay_notify_rsp_body_t *rsp_body =
				(ipay_notify_rsp_body_t *) rsp_body_buf;
		rsp_body->verify_result = 2;  // 预交易不存在
		rsp_body->trans_id = trans_id;
		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0) {
			ERROR_LOG("set autocommit true error = %s",
					mysql_error(mysql_conn));
		}

		return 0;
	}TRACE_LOG("ipay_notify 26");
	int status = atoi_safe(row[1]);
	if (status >= RECEIPT_DEALED) {
		*status_code = 0;
		*rsp_body_len = sizeof(ipay_notify_rsp_body_t);
		ipay_notify_rsp_body_t *rsp_body =
				(ipay_notify_rsp_body_t *) rsp_body_buf;
		rsp_body->verify_result = 1;  // 交易已经处理
		rsp_body->trans_id = trans_id;
		ret = mysql_autocommit(mysql_conn, true);
		if (ret != 0) {
			ERROR_LOG("set autocommit true error = %s",
					mysql_error(mysql_conn));
		}
		return 0;
	}
	int id = atoi_safe(row[0]);
	// 安全性校验
	if (id != pay_log_id) {
		ERROR_LOG("receipt_paylog_id %d != pay_log_id %d", id, pay_log_id);
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	int price = atoi_safe(row[2]);
	if (price != amount) {
		ERROR_LOG("receipt_paylog_id %d,uid is %d", id, user_id);
		ERROR_LOG("price(%d)is not equal db price(%f)", price, amount);
	}
	int storeid = atoi_safe(row[3]);
	int user_id_db = atoi_safe(row[4]);
	if (user_id_db != user_id) {
		ERROR_LOG("receipt_paylog_id %d", id);
		ERROR_LOG("uid(%d)is not equal db uid(%d)", user_id, user_id_db);
	}
	int costs = 0;
    int item_id = 0;
	int golds_to_add = 0;
    int diamonds_to_add = 0;
    int others_goods_to_add = 0;
    
    
	if (storeid == 1) {
		golds_to_add = 10000;
		costs = 100;
	} else if (storeid == 2) {
		golds_to_add = 50000;
		costs = 500;
	} else if (storeid == 3) {
		golds_to_add = 100000;
		costs = 1000;
	} else if (storeid == 4) {
		golds_to_add = 350000;
		costs = 3000;
	} else if (storeid == 5) {
		golds_to_add = 600000;
		costs = 5000;
	} else if (storeid == 6) {
		golds_to_add = 1250000;
		costs = 10000;
    } else if (storeid == 7) {
        golds_to_add = 2560000;
        costs = 20000;
    } else if (storeid == 8) {
        golds_to_add = 8300000;
        costs = 60000;
    }
    else if (storeid == 7) {
        golds_to_add = 2560000;
        costs = 20000;
    }
    else if (storeid == 8) {
        golds_to_add = 8300000;
        costs = 60000;
    }
    else if (storeid == 9) {
        golds_to_add = 100;
        costs = 1;
    }
    //以下是钻石商品id
    else if (storeid == 10) {
        diamonds_to_add = 50;
        costs = 500;
    }
    else if (storeid == 11) {
        diamonds_to_add = 100;
        costs = 1000;
    }
    else if (storeid == 12) {
        diamonds_to_add = 300;
        costs = 3000;
    }
    else if (storeid == 13) {
        diamonds_to_add = 500;
        costs = 5000;
    }
    else if (storeid == 14) {
        diamonds_to_add = 1000;
        costs = 10000;
    }
    else if (storeid == 15) {
        diamonds_to_add = 2000;
        costs = 20000;
    }
    else if (storeid == 16) {
        diamonds_to_add = 6000;
        costs = 60000;
    }
    //其他商品
    else if (storeid == 101) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 951;
    }
    else if (storeid == 102) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 952;
    }
    else {
		// 未知的商品id
		ERROR_LOG("Unknown pdt id: %d, %d", user_id, storeid);
	}TRACE_LOG("ipay_notify 27");
    
	if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
    {
		ret =
				g_mysql_handler->execsql(
						"UPDATE pay_360_log_table SET golds = %d, diamonds = %u, costs = %u, trans_time = NOW(), status = %d,transno= '%s' WHERE id = %d;",
						golds_to_add, diamonds_to_add, costs, RECEIPT_VERIFIED, order_id.c_str(), id);
		if (ret < 0) {
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
		}TRACE_LOG("ipay_notify 28");
        
        // 360的充值渠道号为
        int channel_id_int = 111;
        
		// 发货
		int32_t result = -1;
		uint64_t beansys_trans_id = 0;
        
        if(golds_to_add > 0)
        {
            if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, id, channel_id_int, &result,
                                        &beansys_trans_id) != 0)
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
            }TRACE_LOG("ipay_notify 29");
        }
        else if(diamonds_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, diamonds_to_add, costs, item_id, id, channel_id_int, &result,                                    &beansys_trans_id) != 0)
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
        else if(others_goods_to_add > 0)
        {
            if (g_diamonds.add_seer_bean(user_id, others_goods_to_add, costs, item_id, id, channel_id_int, &result,                                    &beansys_trans_id) != 0)
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
		ret =
				g_mysql_handler->execsql(
						"UPDATE pay_360_log_table SET beansys_trans_id = %d, deal_time = NOW(), status = %d WHERE id = %d;",
						beansys_trans_id, RECEIPT_DEALED, pay_log_id);
		if (ret < 0) {
			ERROR_LOG("update apple pay log failed, error = %s",
					g_mysql_handler->get_last_errstr());
			*status_code = DB_ERR;

			ret = mysql_commit(mysql_conn);
			if (ret != 0) {
				ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
			}

			ret = mysql_autocommit(mysql_conn, true);
			if (ret != 0) {
				ERROR_LOG("set autocommit true error = %s",
						mysql_error(mysql_conn));
			}

			return -1;
		}TRACE_LOG("ipay_notify 30");
		if (result == BEANSYS_E_TRANS_ALREADY_DEALED) {
			DEBUG_LOG(
					"Add golden bean already dealed, user_id: %d, trans_id: %d",
					user_id, pay_log_id);
			*status_code = 0;
			*rsp_body_len = sizeof(ipay_notify_rsp_body_t);
			ipay_notify_rsp_body_t *ack_body =
					(ipay_notify_rsp_body_t *) rsp_body_buf;
			ack_body->verify_result = 1;  // 交易已处理
			ack_body->trans_id = trans_id;

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
		} else if (result != BEANSYS_S_OK) {
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
	} else {
		TRACE_LOG("ipay_notify 31");
		ret =
				g_mysql_handler->execsql(
						"UPDATE pay_360_log_table SET  golds = %d, diamonds = %d, costs = %d, trans_time = NOW(),deal_time = NOW(), status = %d WHERE id = %d;",
						golds_to_add, diamonds_to_add, costs, RECEIPT_VERIFIED, id);
		if (ret < 0) {
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
	}TRACE_LOG("ipay_notify 32");
	ret = mysql_commit(mysql_conn);
	if (ret != 0) {
		ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
	}TRACE_LOG("ipay_notify 33");
	ret = mysql_autocommit(mysql_conn, true);
	TRACE_LOG("ipay_notify 34");
	if (ret != 0) {
		ERROR_LOG("set autocommit true error = %s", mysql_error(mysql_conn));
	}
	*status_code = 0;
	*rsp_body_len = sizeof(ipay_notify_rsp_body_t);
	ipay_notify_rsp_body_t *rsp_body = (ipay_notify_rsp_body_t *) rsp_body_buf;
	rsp_body->verify_result = 0;  // 处理成功
	rsp_body->trans_id = trans_id;
	return 0;
}

#endif /* CMD_HANDLER_CPP_ */

