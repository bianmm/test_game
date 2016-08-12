#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>

#include "../include/benchapi.h"
#include "../include/proto.h"
#include "../include/functions.h"
#include "../include/i_mysql_iface.h"
#include "../include/appstore.h"
#include "../include/beansys_iface.h"

extern i_mysql_iface *g_mysql_handler;

extern beansys_iface_t g_beansys;
extern beansys_iface_t g_diamonds;
char g_receipt_rsp_buf[10240] = {0};

int apple_preorder(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code)
{
    *ack_body_len = 0;
    if (req_body_len != sizeof(apple_preorder_req_body_t)) {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }

    if (user_id == 0) {
        ERROR_LOG("user_id is zero!");
        *status_code = USER_ID_NOFIND_ERR;
        return -1;
    }

    apple_preorder_req_body_t *req_body =
        (apple_preorder_req_body_t *)req_body_buf;
    char product_id[33] = {0};
    char ud[65] = {0};
    memcpy(product_id, req_body->product_id, 32);
    memcpy(ud, req_body->ud, 64);

    trim_str(product_id);
    trim_str(ud);

    if (strlen(product_id) <= 0) {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    if (strlen(ud) <= 0) {
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
        ERROR_LOG("Connect to diamonds fail, ret(%d)", ret);
        *status_code = NET_ERR;
        return -1;
    }
    
    MYSQL *mysql_conn = NULL;
    mysql_conn = g_mysql_handler->get_conn();
    if (mysql_conn == NULL) {
        ERROR_LOG("mysql connection is null");
        *status_code = SYS_ERR;
        return -1;
    }

    char product_id_mysql[2 * 32 + 1] = {0};
    char ud_mysql[2 * 64 + 1] = {0};
    mysql_real_escape_string(mysql_conn, product_id_mysql,
            product_id, strlen(product_id));
    mysql_real_escape_string(mysql_conn, ud_mysql,
                             ud, strlen(ud));

    ret = g_mysql_handler->execsql("INSERT INTO applepay_log_table (uid, product_id, ud, preorder_time) VALUES (%d, '%s', '%s', NOW()) ON DUPLICATE KEY UPDATE uid = %d, product_id = '%s', ud = '%s', preorder_time = NOW()",
            user_id, product_id_mysql, ud_mysql, user_id, product_id_mysql, ud_mysql);
    if (ret < 0) {
        ERROR_LOG("insert (%d, %s, %s) failed, error = %s",
                user_id, product_id_mysql, ud_mysql,
                g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }

    if (ret == 0) {
        // 记录已经存在
        ERROR_LOG("insert (%d, %s, %s) affected 0 row",
                  user_id, product_id_mysql, ud_mysql);
        *status_code = DB_ERR;
        return -1;
    } else {
       // 插入成功
    }
    
    long trans_id = mysql_insert_id(mysql_conn);

    *status_code = 0;
    *ack_body_len = sizeof(apple_preorder_ack_body_t);
    apple_preorder_ack_body_t *ack_body =
    (apple_preorder_ack_body_t *)ack_body_buf;
    memcpy(ack_body->product_id, req_body->product_id, sizeof(ack_body->product_id));
    ack_body->trans_id = trans_id;

    return 0;
}

int receipt_verify_for_preorder_zero(uint32_t user_id,
                   const char *req_body_buf,
                   int req_body_len,
                   char *ack_body_buf,
                   int *ack_body_len,
                   uint32_t *status_code)
{
    *ack_body_len = 0;
    if (req_body_len <= (int)sizeof(receipt_verify_req_body_t)) {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }
    
    if (user_id == 0) {
        *status_code = USER_ID_NOFIND_ERR;
        return -1;
    }
    
    receipt_verify_req_body_t *req_body =
    (receipt_verify_req_body_t *)req_body_buf;
    
    if (req_body->trans_id < 0) {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    if (req_body->receipt_len >= 8192) {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    char receipt[8192] = {0};
    memcpy(receipt, req_body->receipt, req_body->receipt_len);
    
    trim_str(receipt);
    
    if (strlen(receipt) <= 0) {
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
    
    int pay_log_id = req_body->trans_id;
    
    MYSQL_ROW row;
    int ret = 0;
    
    // 检查回执是否已经校验
    unsigned char md5_buffer_16[16];
    unsigned char md5_buffer_32[32];
    MD5(reinterpret_cast<unsigned char *>(receipt), strlen(receipt), md5_buffer_16);
    md5_16to32(md5_buffer_16, md5_buffer_32, 0);
    char receipt_md5[33] = {'\0'};
    memcpy(receipt_md5, md5_buffer_32, 32);
    
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT verify_result, receipt_pdt_id, id, pay_log_id FROM apple_receipt_log_table WHERE receipt_hash = '%s'", receipt_md5);
    if (ret < 0) {
        ERROR_LOG("get receipt error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }
    
    int verify_result = -1;
    char receipt_pdt_id[64] = {0};
    long receipt_dbid = 0;
    int receipt_paylog_id = 0;
    if (ret > 0) {
        verify_result = atoi_safe(row[0]);
        strcpy(receipt_pdt_id, row[1]);
        receipt_dbid = atoi_safe(row[2]);
        receipt_paylog_id = atoi_safe(row[3]);
        
        // 安全性校验
        if (receipt_paylog_id != pay_log_id) {
            CRIT_LOG("receipt_paylog_id %d != pay_log_id %d, hash: %s", receipt_paylog_id, pay_log_id, receipt_md5);
            *status_code = CMD_PARAM_ERR;
            return -1;
        }
    }
    
    // 验证回执
    if (verify_result < 0) {
        memset(g_receipt_rsp_buf, 0, sizeof(g_receipt_rsp_buf));
        verify_result = Get_Receipt_Status_curl(receipt, g_receipt_rsp_buf, sizeof(g_receipt_rsp_buf), receipt_pdt_id, sizeof(receipt_pdt_id), false);
        if (verify_result == 21007) {
            DEBUG_LOG("Receipt is not pdt, try in sandbox");
            memset(g_receipt_rsp_buf, 0, sizeof(g_receipt_rsp_buf));
            verify_result = Get_Receipt_Status_curl(receipt, g_receipt_rsp_buf, sizeof(g_receipt_rsp_buf), receipt_pdt_id, sizeof(receipt_pdt_id), true);
        }
        
        // 注，因为存在多个进程同时处理一个请求的问题，而且苹果对于并发校验貌似只会有1个返回成功
        // 所以一旦这个回执验证成功，其它处理就不覆盖
        //ret = g_mysql_handler->execsql("INSERT INTO apple_receipt_log_table (pay_log_id, receipt_hash, receipt, verify_resp, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE verify_resp = '%s', verify_result = %d, receipt_pdt_id = '%s';", pay_log_id, receipt_md5, receipt, g_receipt_rsp_buf, verify_result, receipt_pdt_id, g_receipt_rsp_buf, verify_result, receipt_pdt_id);
        ret = g_mysql_handler->execsql("INSERT INTO apple_receipt_log_table (pay_log_id, receipt_hash, receipt, verify_resp, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE verify_resp = IF(verify_result >= 0, verify_resp, '%s'), receipt_pdt_id = IF(verify_result >= 0, receipt_pdt_id, '%s'), verify_result = IF(verify_result >= 0, verify_result, %d);", pay_log_id, receipt_md5, receipt, g_receipt_rsp_buf, verify_result, receipt_pdt_id, g_receipt_rsp_buf, receipt_pdt_id, verify_result);
        if (ret < 0) {
            ERROR_LOG("update apple receipt failed, error = %s",
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            return -1;
        }
        
        receipt_dbid = mysql_insert_id(mysql_conn);
    }
    
    // 相同订单排队处理
    ret = mysql_autocommit(mysql_conn, false);
    if (ret != 0) {
        ERROR_LOG("set autocommit false failed, error = %s",
                  mysql_error(mysql_conn));
        *status_code = DB_ERR;
        return -1;
    }
    
    // 判断是否已经处理过
    // 没有预交易的表用receipt_id做主键
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT status, product_id, uid, id FROM applepay_log_no_preorder_table WHERE receipt_id = %d FOR UPDATE", receipt_dbid);
    if (ret < 0) {
        ERROR_LOG("get status error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        
        return -1;
    }
    
    int pay_log_no_preorder_id = 0;
    if (ret <= 0) {
        // 交易记录不存在，插入一条新的
        char product_id_mysql[2 * 32 + 1] = {0};
        mysql_real_escape_string(mysql_conn, product_id_mysql,
                                 receipt_pdt_id, strlen(receipt_pdt_id));
        
        ret = g_mysql_handler->execsql("INSERT INTO applepay_log_no_preorder_table (receipt_id, uid, product_id) VALUES (%d, %d, '%s')",
                                       receipt_dbid, user_id, product_id_mysql);
        if (ret < 0) {
            ERROR_LOG("insert (%d, %s) failed, error = %s",
                      user_id, product_id_mysql,
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            return -1;
        }
        
        if (ret == 0) {
            // 记录已经存在
            ERROR_LOG("insert (%d, %s) affected 0 row",
                      user_id, product_id_mysql);
            *status_code = DB_ERR;
            return -1;
        } else {
            // 插入成功
        }
        
        pay_log_no_preorder_id = mysql_insert_id(mysql_conn);
    }
    else {
        // 交易记录存在，判断是否已处理
        int status = atoi_safe(row[0]);
        if ((status >= RECEIPT_DEALED) && (verify_result >= 0)) {
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 1;  // 交易已处理
            ack_body->trans_id = req_body->trans_id;
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
        
        // 判断是否和预交易是同一个用户
        int uid_db = atoi_safe(row[2]);
        if ((int)user_id != uid_db) {
            DEBUG_LOG("receipt uid %d != preorder uid %d, trans id: %d", user_id, uid_db, req_body->trans_id);
            
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 5;  // 回执校验用户和预交易用户不一致
            ack_body->trans_id = req_body->trans_id;
            
            ret = mysql_autocommit(mysql_conn, true);
            if (ret != 0) {
                ERROR_LOG("set autocommit true error = %s",
                          mysql_error(mysql_conn));
            }
            
            return -1;
        }
        
        pay_log_no_preorder_id = atoi_safe(row[3]);
    }

    // 更新交易记录
    if (verify_result != 0) {
        if (verify_result < 0) {
            // 回执校验失败，下次请求时再次校验
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 4;  // 回执校验失败
            ack_body->trans_id = req_body->trans_id;
        }
        else {
            // 回执无效
            ret = g_mysql_handler->execsql("UPDATE applepay_log_no_preorder_table SET trans_time = NOW(), deal_time = NOW(), status = %d WHERE receipt_id = %d;", RECEIPT_DEALED, receipt_dbid);
            if (ret < 0) {
                ERROR_LOG("update apple pay log failed, error = %s",
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
            
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 3;  // 回执无效
            ack_body->trans_id = req_body->trans_id;
        }
        
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
    else {
        int costs = 0;
        int item_id = 0;
        int golds_to_add = 0;
        int diamonds_to_add = 0;
        int others_goods_to_add = 0;
        
        if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.1w") == 0)) {
            golds_to_add = 10000;
            costs = 100;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.6w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.6w") == 0)) {
            golds_to_add = 60000;
            costs = 600;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.12w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.12w") == 0)) {
            golds_to_add = 120000;
            costs = 1200;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.35w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.35w") == 0)) {
            golds_to_add = 350000;
            costs = 3000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.60w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.60w") == 0)) {
            golds_to_add = 600000;
            costs = 5000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.160w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.160w") == 0)) {
            golds_to_add = 1600000;
            costs = 12800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.330w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.330w") == 0)) {
            golds_to_add = 3300000;
            costs = 25800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.900w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.900w") == 0)) {
            golds_to_add = 9000000;
            costs = 64800;
        }
        
        //钻石
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.60zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.60zuan") == 0)) {
            diamonds_to_add = 60;
            costs = 600;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.120zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.120zuan") == 0)) {
            diamonds_to_add = 120;
            costs = 1200;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.300zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.300zuan") == 0)) {
            diamonds_to_add = 300;
            costs = 3000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.500zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.500zuan") == 0)) {
            diamonds_to_add = 500;
            costs = 5000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.1280zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.1280zuan") == 0)) {
            diamonds_to_add = 1280;
            costs = 12800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.2580zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.2580zuan") == 0)) {
            diamonds_to_add = 2580;
            costs = 25800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.6480zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.6480zuan") == 0)) {
            diamonds_to_add = 6480;
            costs = 64800;
        }
        //其他商品
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.hbplb2") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplb2") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplbxh") == 0)) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.dcplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.dcplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.newlibao") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.dcplbxh") == 0)) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 952;
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %s", user_id, receipt_pdt_id);
        }
        
        if (golds_to_add > 0  || diamonds_to_add > 0 || others_goods_to_add > 0) {
            ret = g_mysql_handler->execsql("UPDATE applepay_log_no_preorder_table SET costs = %d, golds = %d, trans_time = NOW(), status = %d WHERE receipt_id = %d;", costs, golds_to_add, RECEIPT_VERIFIED, receipt_dbid);
            if (ret < 0) {
                ERROR_LOG("update apple pay log failed, error = %s",
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
            
            // 苹果的充值渠道号为
            int channel_id_int = 2;
            
            // 发货
            int32_t result = -1;
            uint64_t beansys_trans_id = 0;
            
            if(golds_to_add > 0)
            {
                if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, pay_log_no_preorder_id, channel_id_int, &result, &beansys_trans_id) != 0)
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
                if (g_diamonds.add_seer_bean(user_id, diamonds_to_add, costs, item_id, pay_log_no_preorder_id, channel_id_int, &result, &beansys_trans_id) != 0)
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
                if (g_diamonds.add_seer_bean(user_id, others_goods_to_add, costs, item_id, pay_log_no_preorder_id, channel_id_int, &result, &beansys_trans_id) != 0)
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

            
            ret = g_mysql_handler->execsql("UPDATE applepay_log_no_preorder_table SET beansys_trans_id = %d, deal_time = NOW(), status = %d WHERE receipt_id = %d;", beansys_trans_id, RECEIPT_DEALED, receipt_dbid);
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
            
            if (result == BEANSYS_E_TRANS_ALREADY_DEALED) {
                DEBUG_LOG("Add golden bean already dealed, user_id: %d, trans_id: %d", user_id, pay_log_no_preorder_id);
                *status_code = 0;
                *ack_body_len = sizeof(receipt_verify_rsp_body_t);
                receipt_verify_rsp_body_t *ack_body =
                (receipt_verify_rsp_body_t *)ack_body_buf;
                ack_body->verify_result = 1;  // 交易已处理
                ack_body->trans_id = req_body->trans_id;
                
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
                
                return 0;
            }
            else if (result != BEANSYS_S_OK) {
                ERROR_LOG("Add golden bean fail, result: %d, %d", user_id, result);
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
        else {
            ret = g_mysql_handler->execsql("UPDATE applepay_log_no_preorder_table SET costs = %d, golds = %d, trans_time = NOW(), deal_time = NOW(), status = %d WHERE receipt_id = %d;", costs, golds_to_add, RECEIPT_DEALED, receipt_dbid);
            if (ret < 0) {
                ERROR_LOG("update apple pay log failed, error = %s",
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
    }
    
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
    
    *status_code = 0;
    *ack_body_len = sizeof(receipt_verify_rsp_body_t);
    receipt_verify_rsp_body_t *ack_body =
    (receipt_verify_rsp_body_t *)ack_body_buf;
    ack_body->verify_result = 0;  // 处理成功
    ack_body->trans_id = req_body->trans_id;
    
    return 0;
}

int receipt_verify(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code)
{
    *ack_body_len = 0;
    if (req_body_len <= (int)sizeof(receipt_verify_req_body_t)) {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }

    if (user_id == 0) {
        *status_code = USER_ID_NOFIND_ERR;
        return -1;
    }

    receipt_verify_req_body_t *req_body =
        (receipt_verify_req_body_t *)req_body_buf;
    
    // 允许trans_id = 0，暂时容许因客户端BUG产生的充值问题
    if (req_body->trans_id == 0) {
        return receipt_verify_for_preorder_zero(user_id, req_body_buf, req_body_len, ack_body_buf, ack_body_len, status_code);
    }
    
    if (req_body->trans_id <= 0) {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    if (req_body->receipt_len >= 8192) {
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    char receipt[8192] = {0};
    memcpy(receipt, req_body->receipt, req_body->receipt_len);

    trim_str(receipt);
    
    if (strlen(receipt) <= 0) {
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
    
    int pay_log_id = req_body->trans_id;
    
    MYSQL_ROW row;
    int ret = 0;
    
    // 检查回执是否已经校验
    unsigned char md5_buffer_16[16];
    unsigned char md5_buffer_32[32];
    MD5(reinterpret_cast<unsigned char *>(receipt), strlen(receipt), md5_buffer_16);
    md5_16to32(md5_buffer_16, md5_buffer_32, 0);
    char receipt_md5[33] = {'\0'};
    memcpy(receipt_md5, md5_buffer_32, 32);
    
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT verify_result, receipt_pdt_id, id, pay_log_id FROM apple_receipt_log_table WHERE receipt_hash = '%s'", receipt_md5);
    if (ret < 0)
    {
        ERROR_LOG("get receipt error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }
    
    int verify_result = -1;
    char receipt_pdt_id[64] = {0};
    long receipt_dbid = 0;
    int receipt_paylog_id = 0;
    if (ret > 0) {
        verify_result = atoi_safe(row[0]);
        strcpy(receipt_pdt_id, row[1]);
        receipt_dbid = atoi_safe(row[2]);
        receipt_paylog_id = atoi_safe(row[3]);
        
        // 安全性校验
        if (receipt_paylog_id != pay_log_id) {
            CRIT_LOG("receipt_paylog_id %d != pay_log_id %d, hash: %s", receipt_paylog_id, pay_log_id, receipt_md5);
            *status_code = CMD_PARAM_ERR;
            return -1;
        }
    }
    
    // 验证回执
    if (verify_result < 0) {
        memset(g_receipt_rsp_buf, 0, sizeof(g_receipt_rsp_buf));
        verify_result = Get_Receipt_Status_curl(receipt, g_receipt_rsp_buf, sizeof(g_receipt_rsp_buf), receipt_pdt_id, sizeof(receipt_pdt_id), false);
        DEBUG_LOG("pdt result: %d", verify_result);
        if (verify_result == 21007) { // Relese
        //if (verify_result != 0) { // Debug
            DEBUG_LOG("Receipt is not pdt, try in sandbox");
            memset(g_receipt_rsp_buf, 0, sizeof(g_receipt_rsp_buf));
            verify_result = Get_Receipt_Status_curl(receipt, g_receipt_rsp_buf, sizeof(g_receipt_rsp_buf), receipt_pdt_id, sizeof(receipt_pdt_id), true);
        }
        
        // 注，因为存在多个进程同时处理一个请求的问题，而且苹果对于并发校验貌似只会有1个返回成功
        // 所以一旦这个回执验证成功，其它处理就不覆盖
        //ret = g_mysql_handler->execsql("INSERT INTO apple_receipt_log_table (pay_log_id, receipt_hash, receipt, verify_resp, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE verify_resp = '%s', verify_result = %d, receipt_pdt_id = '%s';", pay_log_id, receipt_md5, receipt, g_receipt_rsp_buf, verify_result, receipt_pdt_id, g_receipt_rsp_buf, verify_result, receipt_pdt_id);
        ret = g_mysql_handler->execsql("INSERT INTO apple_receipt_log_table (pay_log_id, receipt_hash, receipt, verify_resp, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE verify_resp = IF(verify_result >= 0, verify_resp, '%s'), receipt_pdt_id = IF(verify_result >= 0, receipt_pdt_id, '%s'), verify_result = IF(verify_result >= 0, verify_result, %d);", pay_log_id, receipt_md5, receipt, g_receipt_rsp_buf, verify_result, receipt_pdt_id, g_receipt_rsp_buf, receipt_pdt_id, verify_result);
        if (ret < 0) {
            ERROR_LOG("update apple receipt failed, error = %s",
                      g_mysql_handler->get_last_errstr());
            *status_code = DB_ERR;
            return -1;
        }
        
        receipt_dbid = mysql_insert_id(mysql_conn);
    }
    
    // 相同订单排队处理
    ret = mysql_autocommit(mysql_conn, false);
    if (ret != 0) {
        ERROR_LOG("set autocommit false failed, error = %s",
                  mysql_error(mysql_conn));
        *status_code = DB_ERR;
        return -1;
    }

    // 判断是否已经处理过
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT status, product_id, uid FROM applepay_log_table WHERE id = %d FOR UPDATE", req_body->trans_id);
    if (ret < 0) {
        ERROR_LOG("get status error = %s",
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
        ERROR_LOG("fatal error, preorder trans not exists, %d, %s", req_body->trans_id, receipt);
        *status_code = 0;
        *ack_body_len = sizeof(receipt_verify_rsp_body_t);
        receipt_verify_rsp_body_t *ack_body =
        (receipt_verify_rsp_body_t *)ack_body_buf;
        ack_body->verify_result = 2;  // 预交易不存在
        ack_body->trans_id = req_body->trans_id;

        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }

        return 0;
    }
    
    int status = atoi_safe(row[0]);
    if ((status >= RECEIPT_DEALED) && (verify_result >= 0)) {
        *status_code = 0;
        *ack_body_len = sizeof(receipt_verify_rsp_body_t);
        receipt_verify_rsp_body_t *ack_body =
        (receipt_verify_rsp_body_t *)ack_body_buf;
        ack_body->verify_result = 1;  // 交易已处理
        ack_body->trans_id = req_body->trans_id;
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }

        return -1;
    }
    
    char preorder_pdt_id[64] = {0};
    strcpy(preorder_pdt_id, row[1]);
    
    // 判断是否和预交易是同一个用户
    int uid_db = atoi_safe(row[2]);
    if ((int)user_id != uid_db) {
        DEBUG_LOG("receipt uid %d != preorder uid %d, trans id: %d", user_id, uid_db, req_body->trans_id);
        
        *status_code = 0;
        *ack_body_len = sizeof(receipt_verify_rsp_body_t);
        receipt_verify_rsp_body_t *ack_body =
        (receipt_verify_rsp_body_t *)ack_body_buf;
        ack_body->verify_result = 5;  // 回执校验用户和预交易用户不一致
        ack_body->trans_id = req_body->trans_id;
        
        ret = mysql_autocommit(mysql_conn, true);
        if (ret != 0) {
            ERROR_LOG("set autocommit true error = %s",
                      mysql_error(mysql_conn));
        }
        
        return -1;
    }
    
    // 更新交易记录
    if (verify_result != 0) {
        if (verify_result < 0) {
            // 回执校验失败，下次请求时再次校验
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 4;  // 回执校验失败
            ack_body->trans_id = req_body->trans_id;
        }
        else {
            // 回执无效
            ret = g_mysql_handler->execsql("UPDATE applepay_log_table SET receipt_id = %d, trans_time = NOW(), deal_time = NOW(), status = %d WHERE id = %d;", receipt_dbid, RECEIPT_DEALED, pay_log_id);
            if (ret < 0) {
                ERROR_LOG("update apple pay log failed, error = %s",
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
            
            *status_code = 0;
            *ack_body_len = sizeof(receipt_verify_rsp_body_t);
            receipt_verify_rsp_body_t *ack_body =
            (receipt_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 3;  // 回执无效
            ack_body->trans_id = req_body->trans_id;
        }
        
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
    else {
        int costs = 0;
        int item_id = 0;
        int golds_to_add = 0;
        int diamonds_to_add = 0;
        int others_goods_to_add = 0;
        
        if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.1w") == 0)) {
            golds_to_add = 10000;
            costs = 100;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.6w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.6w") == 0)) {
            golds_to_add = 60000;
            costs = 600;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.12w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.12w") == 0)) {
            golds_to_add = 120000;
            costs = 1200;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.35w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.35w") == 0)) {
            golds_to_add = 350000;
            costs = 3000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.60w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.60w") == 0)) {
            golds_to_add = 600000;
            costs = 5000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.160w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.160w") == 0)) {
            golds_to_add = 1600000;
            costs = 12800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.330w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.330w") == 0)) {
            golds_to_add = 3300000;
            costs = 25800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.900w") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.900w") == 0)) {
            golds_to_add = 9000000;
            costs = 64800;
        }
        
        //钻石
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.60zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.60zuan") == 0)) {
            diamonds_to_add = 60;
            costs = 600;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.120zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.120zuan") == 0)) {
            diamonds_to_add = 120;
            costs = 1200;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.300zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.300zuan") == 0)) {
            diamonds_to_add = 300;
            costs = 3000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.500zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.500zuan") == 0)) {
            diamonds_to_add = 500;
            costs = 5000;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.1280zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.1280zuan") == 0)) {
            diamonds_to_add = 1280;
            costs = 12800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.2580zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.2580zuan") == 0)) {
            diamonds_to_add = 2580;
            costs = 25800;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.6480zuan") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.6480zuan") == 0)) {
            diamonds_to_add = 6480;
            costs = 64800;
        }
        //其他商品
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.hbplb2") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplb2") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.hbplbxh") == 0)) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else if ((strcasecmp(receipt_pdt_id, "com.junyou.buyu.dcplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyuen.dcplb") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.newlibao") == 0) ||
                 (strcasecmp(receipt_pdt_id, "com.junyou.buyu.dcplbxh") == 0)) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 952;
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %s", user_id, receipt_pdt_id);
        }
        
        if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
        {
            ret = g_mysql_handler->execsql("UPDATE applepay_log_table SET costs = %d, golds = %d, diamonds = %d, receipt_id = %d, trans_time = NOW(), status = %d WHERE id = %d;", costs, golds_to_add, diamonds_to_add, receipt_dbid, RECEIPT_VERIFIED, pay_log_id);
            if (ret < 0)
            {
                ERROR_LOG("update apple pay log failed, error = %s",
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
            
            // 苹果的充值渠道号为
            int channel_id_int = 2;
            
            // 发货
            int32_t result = -1;
            uint64_t beansys_trans_id = 0;
            
            if(golds_to_add > 0)
            {
                if (g_beansys.add_seer_bean(user_id, golds_to_add, costs, item_id, pay_log_id, channel_id_int, &result, &beansys_trans_id) != 0)
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
                if (g_diamonds.add_seer_bean(user_id, diamonds_to_add, costs, item_id, pay_log_id, channel_id_int, &result, &beansys_trans_id) != 0)
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
                if (g_diamonds.add_seer_bean(user_id, others_goods_to_add, costs, item_id, pay_log_id, channel_id_int, &result, &beansys_trans_id) != 0)
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
            
            ret = g_mysql_handler->execsql("UPDATE applepay_log_table SET beansys_trans_id = %d, deal_time = NOW(), status = %d WHERE id = %d;", beansys_trans_id, RECEIPT_DEALED, pay_log_id);
            if (ret < 0)
            {
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
            
            if (result == BEANSYS_E_TRANS_ALREADY_DEALED)
            {
                DEBUG_LOG("Add golden bean already dealed, user_id: %d, trans_id: %d", user_id, pay_log_id);
                *status_code = 0;
                *ack_body_len = sizeof(receipt_verify_rsp_body_t);
                receipt_verify_rsp_body_t *ack_body =
                (receipt_verify_rsp_body_t *)ack_body_buf;
                ack_body->verify_result = 1;  // 交易已处理
                ack_body->trans_id = req_body->trans_id;
                
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

                return 0;
            }
            else if (result != BEANSYS_S_OK)
            {
                ERROR_LOG("Add golden bean fail, result: %d, %d", user_id, result);
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
            ret = g_mysql_handler->execsql("UPDATE applepay_log_table SET costs = %d, golds = %d, diamonds = %d, receipt_id = %d, trans_time = NOW(), deal_time = NOW(), status = %d WHERE id = %d;", costs, golds_to_add, diamonds_to_add, receipt_dbid, RECEIPT_DEALED, pay_log_id);
            if (ret < 0)
            {
                ERROR_LOG("update apple pay log failed, error = %s",
                          g_mysql_handler->get_last_errstr());
                *status_code = DB_ERR;
                
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
    }
    
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

    *status_code = 0;
    *ack_body_len = sizeof(receipt_verify_rsp_body_t);
    receipt_verify_rsp_body_t *ack_body =
    (receipt_verify_rsp_body_t *)ack_body_buf;
    ack_body->verify_result = 0;  // 处理成功
    ack_body->trans_id = req_body->trans_id;

    return 0;
}

