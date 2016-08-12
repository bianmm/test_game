#ifndef CMD_HANDLER_CPP_
#define CMD_HANDLER_CPP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json/json.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>

#include "../include/beansys_iface.h"
#include "../include/benchapi.h"
#include "../include/functions.h"
#include "../include/i_mysql_iface.h"
#include "../include/ipay.h"
#include "../include/proto.h"
extern i_mysql_iface *g_mysql_handler;
extern beansys_iface_t g_beansys;
extern beansys_iface_t g_diamonds;
char g_receipt_rsp_buf[10240] = { 0 };

int ipay_preorder(uint32_t user_id, const char *req_body_buf, int req_body_len,
		char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code) {
	*rsp_body_len = 0;
	if (req_body_len != sizeof(ipay_preorder_req_body_t)) {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}

	if (user_id == 0) {
		ERROR_LOG("user_id is zero!");
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	int ret = g_beansys.ping();
	if (ret != 0) {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}
    
    ret = g_diamonds.ping();
    if (ret != 0) {
        ERROR_LOG("Connect to dianmods fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
    
	ipay_preorder_req_body_t *req_body =
			(ipay_preorder_req_body_t *) req_body_buf;
	char ud[sizeof(req_body->ud) + 1] = { 0 };
	memcpy(ud, req_body->ud, sizeof(req_body->ud));

	trim_str(ud);
	if (strlen(ud) <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	MYSQL *mysql_conn = NULL;
	mysql_conn = g_mysql_handler->get_conn();
	if (mysql_conn == NULL) {
		ERROR_LOG("mysql connection is null");
		*status_code = SYS_ERR;
		return -1;
	}

	char ud_mysql[2 * sizeof(req_body->ud) + 1] = { 0 };
	mysql_real_escape_string(mysql_conn, ud_mysql, ud, strlen(ud));
	ret =
			g_mysql_handler->execsql(
					"INSERT INTO ipay_log_table (uid, ud,store_id,price) VALUES (%d, '%s', %d,%d) ON DUPLICATE KEY UPDATE uid = %d, ud = '%s',store_id = %d,  price = %d",
					user_id, ud_mysql, req_body->storeid, req_body->price,
					user_id, ud_mysql, req_body->storeid, req_body->price);
	if (ret < 0) {
		ERROR_LOG("insert (%d, '%s','%s', %d,%d) failed, error = %s", user_id,
				ud_mysql, req_body->storeid, req_body->price,
				g_mysql_handler->get_last_errstr());
		*status_code = DB_ERR;
		return -1;
	}

	if (ret == 0) {
		// 记录已经存在
		ERROR_LOG("insert (%d, '%s','%s', %d,%d) affected 0 row", user_id,
				ud_mysql, req_body->storeid, req_body->price);
		*status_code = DB_ERR;
		return -1;
	} else {
		// 插入成功
	}

	long trans_id = mysql_insert_id(mysql_conn);

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

int ipay_notify(uint32_t userid, const char *req_body_buf, int req_body_len,
		char *rsp_body_buf, int *rsp_body_len, uint32_t *status_code) {
	DEBUG_LOG("ipay_notify");
	*rsp_body_len = 0;
	if (req_body_len <= (int) sizeof(ipay_notify_req_body_t)) {
		*status_code = PROTO_LEN_ERR;
		return -1;
	}

	int ret = g_beansys.ping();
	if (ret != 0) {
		ERROR_LOG("Connect to beansys fail, ret(%d)", ret);
		*status_code = NET_ERR;
		return -1;
	}
    ret = g_diamonds.ping();
    if (ret != 0) {
        ERROR_LOG("Connect to dianmods fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }

	ipay_notify_req_body_t *req_body = (ipay_notify_req_body_t *) req_body_buf;
	DEBUG_LOG("sign_len is %u", req_body->sign_len);
	DEBUG_LOG("transdata_len is %u", req_body->transdata_len);
	if (req_body->sign_len >= MAX_SNDBUF_LEN
			|| req_body->transdata_len >= MAX_SNDBUF_LEN) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	char sign[MAX_SNDBUF_LEN] = { 0 };
	char transdata[MAX_SNDBUF_LEN] = { 0 };

	memcpy(sign, req_body->data, req_body->sign_len);
	memcpy(transdata, req_body->data + req_body->sign_len,
			req_body->transdata_len);
	DEBUG_LOG("sign_len is %u", req_body->sign_len);
	DEBUG_LOG("sign is %s", sign);
	DEBUG_LOG("transdata_len is %u", req_body->transdata_len);
	DEBUG_LOG("transdata is %s", transdata);
	if (strlen(sign) <= 0 || strlen(transdata) <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	MYSQL *mysql_conn = NULL;
	mysql_conn = g_mysql_handler->get_conn();
	if (mysql_conn == NULL) {
		ERROR_LOG("mysql connection is null while confirm apple pay");
		*status_code = SYS_ERR;
		return -1;
	}
	//验签
	if (!ipay_sign_verify(transdata, sign)) {
		// 验签失败，尝试用三格合作的验签
        ERROR_LOG("ipay sign verify fail, try in sange");
        if (!ipay_sign_verify_sange(transdata, sign)) {
            // 验签失败，尝试用frankwu
            if (!ipay_sign_verify_frankwu(transdata, sign)) {
                ERROR_LOG("ipay sign verify fail, all failed");
                *status_code = 0;
                *rsp_body_len = sizeof(ipay_notify_rsp_body_t);
                ipay_notify_rsp_body_t *rsp_body =
                (ipay_notify_rsp_body_t *) rsp_body_buf;
                rsp_body->verify_result = 3;  // 验签失败
                rsp_body->trans_id = 0;
                return 0;
            }
            else {
                DEBUG_LOG("frankwu sign verify success");
            }
        }
        else {
            DEBUG_LOG("sange sign verify success");
        }
	}
	std::vector<std::string> vec;
	std::string cporderid;
	std::string transid;
	std::string user_id_str;
	int feetype = -1;
	double money = -1;
	int result = -1;
	int transtype = -1;
	json_object * jobj = json_tokener_parse(transdata);
	enum json_type type;
	json_object_object_foreach(jobj, key, val)
	{ /*Passing through every array element*/
		type = json_object_get_type(val);
		switch (type) {
		case json_type_int:
			if (strcmp(key, "feetype") == 0) {
				feetype = json_object_get_int(val);
			}
			if (strcmp(key, "result") == 0) {
				result = json_object_get_int(val);
			}
			if (strcmp(key, "transtype") == 0) {
				transtype = json_object_get_int(val);
			}
			break;
		case json_type_string:
			if (strcmp(key, "appuserid") == 0) {
				user_id_str = json_object_get_string(val);
			}
			if (strcmp(key, "cporderid") == 0) {
				cporderid = json_object_get_string(val);
			}
			if (strcmp(key, "transid") == 0) {
				transid = json_object_get_string(val);
			}
			break;
		case json_type_double:
			if (strcmp(key, "money") == 0) {
				money = json_object_get_double(val);
			}
			break;
		}
	}
	int user_id = atoi_safe(user_id_str.c_str());
	if (user_id == 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	DEBUG_LOG("trans_id is %s", cporderid.c_str());
	int trans_id = atoi_safe(cporderid.c_str());
	if (trans_id <= 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}
	if (result != 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	if (transtype != 0) {
		*status_code = CMD_PARAM_ERR;
		return -1;
	}

	int pay_log_id = trans_id;
	MYSQL_ROW row;
	ret = 0;
	// 开始事务
	ret = mysql_autocommit(mysql_conn, false);
	if (ret != 0) {
		ERROR_LOG("set autocommit false failed, error = %s",
				mysql_error(mysql_conn));
		*status_code = DB_ERR;
		return -1;
	}

	// 判断是否已经处理过
	ret =
			g_mysql_handler->select_first_row(&row,
					"SELECT id,status, price,store_id,uid FROM ipay_log_table WHERE id = %s FOR UPDATE",
					cporderid.c_str());
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
	}

	if (ret <= 0) {
		// 预交易记录不存在
		ERROR_LOG("fatal error, ipay preorder trans not exists,  %s",
				cporderid.c_str());
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
	}

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
	if (price != money * 100) {
		ERROR_LOG("receipt_paylog_id %d,uid is %d", id, user_id);
		ERROR_LOG("price(%d)is not equal db price(%f)", price, money * 100);
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
	}else if (storeid == 4) {
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
	} else if (storeid == 9) {
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
    else if (storeid == 17) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 951;
    }
    else if (storeid == 18) {
        others_goods_to_add = 1;
        costs = 3000;
        item_id = 952;
    }
    else {
		// 未知的商品id
		ERROR_LOG("Unknown pdt id: %d, %d", user_id, storeid);
	}

	if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
    {
		ret =g_mysql_handler->execsql(
						"UPDATE ipay_log_table SET  golds = %d, diamonds = %d, costs = %d,  trans_time = NOW(), status = %d,transno= '%s' WHERE id = %d;",
						golds_to_add, diamonds_to_add, costs, RECEIPT_VERIFIED, transid.c_str(), id);
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
        
        // 爱贝的支付渠道号为
        int channel_id_int = 101;

		// 发货
		int32_t result = -1;
		uint64_t beansys_trans_id = 0;
        
        if(golds_to_add > 0)
        {
            if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, id, channel_id_int, &result,                                    &beansys_trans_id) != 0)
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
						"UPDATE ipay_log_table SET beansys_trans_id = %d, deal_time = NOW(), status = %d WHERE id = %d;",
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
		}

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
	}
    
    else
    {
		ret =
				g_mysql_handler->execsql(
						"UPDATE ipay_log_table SET  golds = %d, diamonds = %d, costs = %d, trans_time = NOW(),deal_time = NOW(), status = %d WHERE id = %d;",
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
	}

	ret = mysql_commit(mysql_conn);
	if (ret != 0) {
		ERROR_LOG("commit error = %s", mysql_error(mysql_conn));
	}

	ret = mysql_autocommit(mysql_conn, true);
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

