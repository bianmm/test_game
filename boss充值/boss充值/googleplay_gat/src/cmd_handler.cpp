#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>
#include <json/json.h>
#include <memory>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include "../include/benchapi.h"
#include "../include/proto.h"
#include "../include/functions.h"
#include "../include/i_mysql_iface.h"
#include "../include/beansys_iface.h"

extern i_mysql_iface *g_mysql_handler;

extern beansys_iface_t g_beansys;
extern beansys_iface_t g_diamonds;
char g_receipt_rsp_buf[10240] = {0};

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static const char* google_pub_key = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAiJqLW6RonJwnPrQPSVeHBLIJV7L0hkbauCH/4KI49MLqC9mr3YVCYCAdW37EjmlZMnqWZeuX541ky/6My8CpjkgphykCioVu8gJF0WuBekaGk/V/BUZdppbikOdxWU3XtPvREu5b0GEAgCbiTclitkTTfI5H1+6KeaLyocSXnYRpz9e+xez4pPtTvNypLfmqaMhqSpgz/fagam869ZDecyO4kRino7ro55EIw9mloIVw49UCG4N+S3H8X2hlUuEJZ3yl7PQV7bbWgDDigHJCO8zEbV6ZCOPEZ4ZHYFAc2C81WcJCTFmiEZbmcUJN0OmFI6c2ol8jUy3qzx8OCel7VQIDAQAB";

static const char* google_pub_key_com_isocial_fishbossvn = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAhua7wiYRHQmF2aEUBbiVSi90rIG/kcxJHLbcQHyik646POUa1GmtWDEIVK1CnVyTrKxR6ojW8th0FL1RyqfvUDvo+ReF+7C3zJdfbMeDT/K3RQWUMAms34TDGQma7mplzSSHmx3GhzXfXOu1jT6T+ZJoDznWE46JCcd61uapXmxePfu55zNTMMnT/pSUwvXT2nFefUytiqUQvnZ90LJHzA8BQJwIROgBIatsYXQw2Tfgo1WZZ1dzQyW2u5zKqHvE9PQCu3mr4XyqWFN3PGONTmds+g+MDMNZ4Iav4Fv5N1poK/AbQRicJ6FHEut10c9LZYqdj2JuDcRGxjS2hJJ06wIDAQAB";

static const char* google_pub_key_com_rms_fbosstw = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvqWGRtqXibqK3xFIOZq8mAiIVkot1L3SCdAFyxgVIeGVUIGYAg60zM1qf9SGIu6jbMhcdk5VTHpARU8lxL80KVpKgTPeRYfg8hHsLfRkgS6Ma7sdkLfyGjWogjLWnbQpFTAOpNpB0+xsUZWWhYlLMHmsLRO7VDLMMLGPixAqLMUKsFcIXgbpVT/4R4CPCPveh17861WU/k6nJmyF+V57wT3jJmwtstFhtDM44ap7zRl9zQ5ZqUqS3v691MVfaAzKbLd2KsDJra642C9it65+km0gIt7s7UrkKmkNlRgfMoUmmPrQqjyHLSO3FhdK4sWJwlWLCAQOlfJUzY3P4arxEQIDAQAB";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string Base64Decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;
    
    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;
        
        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    
    return ret;
}

int Verify_GoogleInappBilling_Signature(const char* data, const char* signature, const char* pub_key_id)
{
    EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
    const EVP_MD* md = EVP_get_digestbyname("SHA1");
    if(NULL == md)
    {
        //std::cout << "err1" << std::endl;
        ERROR_LOG("NULL == md fail");
        EVP_MD_CTX_destroy(mdctx);
        return -1;
    }
    if(0 == EVP_VerifyInit_ex(mdctx, md, NULL))
    {
        //std::cout << "err2" << std::endl;
        ERROR_LOG("EVP_VerifyInit_ex fail");
        EVP_MD_CTX_destroy(mdctx);
        return -1;
    }
    
    if(0 == EVP_VerifyUpdate(mdctx, (void*)data, strlen(data)))
    {
        //std::cout << "err3" << std::endl;
        ERROR_LOG("EVP_VerifyUpdate fail");
        EVP_MD_CTX_destroy(mdctx);
        return -1;
    }
    //!!!!!!!!!!!!!!!!!!!
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64,BIO_FLAGS_BASE64_NO_NL);
    
    BIO* bPubKey = BIO_new(BIO_s_mem());
    BIO_puts(bPubKey,pub_key_id);
    BIO_push(b64, bPubKey);
    
    EVP_PKEY* pubkey = d2i_PUBKEY_bio(b64, NULL);
    //!!!!!!!!!!!!!!!!!!!
    std::string decoded_signature = Base64Decode(std::string(signature));
    //std::cout << "sig: " << (unsigned char*)decoded_signature.c_str() << std::endl;
    //std::cout << "sig len: " << decoded_signature.length() << std::endl;
    
    int rst = EVP_VerifyFinal(mdctx, (unsigned char*)decoded_signature.c_str(), decoded_signature.length(), pubkey);
    
    if (rst < 0) {
        int err = ERR_get_error();
        ERROR_LOG("EVP_VerifyFinal fail, %d: %s", err, ERR_error_string(err, NULL));
    }
    
    EVP_MD_CTX_destroy(mdctx);
    BIO_free(b64);
    BIO_free(bPubKey);
    EVP_PKEY_free(pubkey);
    
    return rst;
}

int googleplay_preorder(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code)
{
    *ack_body_len = 0;
    if (req_body_len != sizeof(googleplay_preorder_req_body_t)) {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }

    if (user_id == 0) {
        ERROR_LOG("user_id is zero!");
        *status_code = USER_ID_NOFIND_ERR;
        return -1;
    }

    googleplay_preorder_req_body_t *req_body =
        (googleplay_preorder_req_body_t *)req_body_buf;
    char ud[65] = {0};
    memcpy(ud, req_body->ud, 64);

    trim_str(ud);

    if (req_body->product_id <= 0) {
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

    char ud_mysql[2 * 64 + 1] = {0};
    mysql_real_escape_string(mysql_conn, ud_mysql,
                             ud, strlen(ud));

    ret = g_mysql_handler->execsql("INSERT INTO googleplay_log_table (uid, product_id, ud, preorder_time) VALUES (%d, '%d', '%s', NOW()) ON DUPLICATE KEY UPDATE uid = %d, product_id = '%d', ud = '%s', preorder_time = NOW()",
            user_id, req_body->product_id, ud_mysql, user_id, req_body->product_id, ud_mysql);
    if (ret < 0) {
        ERROR_LOG("insert (%d, %d, %s) failed, error = %s",
                user_id, req_body->product_id, ud_mysql,
                g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }

    if (ret == 0) {
        // 记录已经存在
        ERROR_LOG("insert (%d, %d, %s) affected 0 row",
                  user_id, req_body->product_id, ud_mysql);
        *status_code = DB_ERR;
        return -1;
    } else {
       // 插入成功
    }
    
    long trans_id = mysql_insert_id(mysql_conn);

    *status_code = 0;
    *ack_body_len = sizeof(googleplay_preorder_ack_body_t);
    googleplay_preorder_ack_body_t *ack_body =
    (googleplay_preorder_ack_body_t *)ack_body_buf;
    ack_body->product_id = req_body->product_id;
    ack_body->trans_id = trans_id;

    return 0;
}

int googleplay_verify(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code)
{
    *ack_body_len = 0;
    if (req_body_len <= (int)sizeof(googleplay_verify_req_body_t)) {
        *status_code = PROTO_LEN_ERR;
        return -1;
    }

    if (user_id == 0) {
        *status_code = USER_ID_NOFIND_ERR;
        return -1;
    }

    googleplay_verify_req_body_t *req_body =
        (googleplay_verify_req_body_t *)req_body_buf;

    if (req_body->trans_id <= 0) {
        ERROR_LOG("invalid trans id: %d", req_body->trans_id);
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    if (req_body->purchase_data_len > 1024) {
        ERROR_LOG("invalid purchase data len: %d", req_body->purchase_data_len);
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    googleplay_verify_req_body_another_part_t* another_part = (googleplay_verify_req_body_another_part_t*)(req_body->purchase_data + req_body->purchase_data_len);
    if (another_part->data_sig_len > 1024) {
        ERROR_LOG("invalid data sig len: %d", another_part->data_sig_len);
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    char purchase_data[1025] = {0};
    memcpy(purchase_data, req_body->purchase_data, req_body->purchase_data_len);
    char data_sig[1025] = {0};
    memcpy(data_sig, another_part->data_sig, another_part->data_sig_len);
    
    trim_str(purchase_data);
    trim_str(data_sig);
    
    if (strlen(purchase_data) <= 0) {
        ERROR_LOG("empty purchase data");
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    if (strlen(data_sig) <= 0) {
        ERROR_LOG("empty data sig");
        *status_code = CMD_PARAM_ERR;
        return -1;
    }
    
    INFO_LOG("googleplay verify, trans_id: %d, purchase_data: %s, data_sig: %s", req_body->trans_id, purchase_data, data_sig);
    
    MYSQL *mysql_conn = NULL;
    mysql_conn = g_mysql_handler->get_conn();
    if (mysql_conn == NULL) {
        ERROR_LOG("mysql connection is null while confirm googleplay pay");
        *status_code = SYS_ERR;
        return -1;
    }
    
    int pay_log_id = req_body->trans_id;
    
    MYSQL_ROW row;
    int ret = 0;
    
    // 检查回执是否已经校验
    unsigned char md5_buffer_16[16];
    unsigned char md5_buffer_32[32];
    MD5(reinterpret_cast<unsigned char *>(data_sig), strlen(data_sig), md5_buffer_16);
    md5_16to32(md5_buffer_16, md5_buffer_32, 0);
    char receipt_md5[33] = {'\0'};
    memcpy(receipt_md5, md5_buffer_32, 32);
    
    ret = g_mysql_handler->select_first_row(&row,
                                            "SELECT verify_result, receipt_pdt_id, id, pay_log_id FROM googleplay_receipt_log_table WHERE data_sig_hash = '%s'", receipt_md5);
    if (ret < 0)
    {
        ERROR_LOG("get receipt error = %s",
                  g_mysql_handler->get_last_errstr());
        *status_code = DB_ERR;
        return -1;
    }
    
    int verify_result = -1;
    char receipt_pdt_id[33] = {0};
    long receipt_dbid = 0;
    int receipt_paylog_id = 0;
    if (ret > 0) {
        verify_result = atoi_safe(row[0]);
        memcpy(receipt_pdt_id, row[1], 32);
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
        
        // 读取当前的packagename
        char pkgname[256] = {0};
        json_object* jobj_pkgname = json_tokener_parse(purchase_data);
        if (jobj_pkgname != NULL) {
            json_object* val_obj_pkgname;
            if (json_object_object_get_ex(jobj_pkgname, "packageName", &val_obj_pkgname)) {
                strcpy(pkgname, json_object_get_string(val_obj_pkgname));
            }
        }
        
        DEBUG_LOG("got pkgname: %s", pkgname);
        if (strcasecmp(pkgname, "com.isocial.fishbossvn") == 0) {
            // 越南版本
            DEBUG_LOG("verify com.isocial.fishbossvn receipt: %s", purchase_data);
            verify_result = Verify_GoogleInappBilling_Signature(purchase_data, data_sig, google_pub_key_com_isocial_fishbossvn);
        }
        else if (strcasecmp(pkgname, "com.rms.fbosstw") == 0) {
            // GP 新版本
            DEBUG_LOG("verify com.rms.fbosstw receipt: %s", purchase_data);
            verify_result = Verify_GoogleInappBilling_Signature(purchase_data, data_sig, google_pub_key_com_rms_fbosstw);
        }
        else {
            DEBUG_LOG("verify normal receipt: %s", purchase_data);
            verify_result = Verify_GoogleInappBilling_Signature(purchase_data, data_sig, google_pub_key);
        }
        
        if (verify_result == 1) {
            // 验证成功时读取商品id
            json_object* jobj = json_tokener_parse(purchase_data);
            if (jobj != NULL) {
                json_object* val_obj;
                int result_code = -1;
                if (json_object_object_get_ex(jobj, "productId", &val_obj)) {
                    strcpy(receipt_pdt_id, json_object_get_string(val_obj));
                }
            }
        }
        
        // 注，因为存在多个进程同时处理一个请求的问题，而且苹果对于并发校验貌似只会有1个返回成功
        // 所以一旦这个回执验证成功，其它处理就不覆盖
        //ret = g_mysql_handler->execsql("INSERT INTO apple_receipt_log_table (pay_log_id, receipt_hash, receipt, verify_resp, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE verify_resp = '%s', verify_result = %d, receipt_pdt_id = '%s';", pay_log_id, receipt_md5, receipt, g_receipt_rsp_buf, verify_result, receipt_pdt_id, g_receipt_rsp_buf, verify_result, receipt_pdt_id);
        ret = g_mysql_handler->execsql("INSERT INTO googleplay_receipt_log_table (pay_log_id, data_sig_hash, purchase_data, data_sig, verify_result, receipt_pdt_id) VALUES (%d, '%s', '%s', '%s', %d, '%s') ON DUPLICATE KEY UPDATE receipt_pdt_id = '%s', verify_result = %d;", pay_log_id, receipt_md5, purchase_data, data_sig, verify_result, receipt_pdt_id, receipt_pdt_id, verify_result);
        if (ret < 0) {
            ERROR_LOG("update googleplay receipt failed, error = %s",
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
                                            "SELECT status, product_id, uid FROM googleplay_log_table WHERE id = %d FOR UPDATE", req_body->trans_id);
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
        ERROR_LOG("fatal error, preorder trans not exists, %d, %d", req_body->trans_id, purchase_data);
        *status_code = 0;
        *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
        googleplay_verify_rsp_body_t *ack_body =
        (googleplay_verify_rsp_body_t *)ack_body_buf;
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
        *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
        googleplay_verify_rsp_body_t *ack_body =
        (googleplay_verify_rsp_body_t *)ack_body_buf;
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
        *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
        googleplay_verify_rsp_body_t *ack_body =
        (googleplay_verify_rsp_body_t *)ack_body_buf;
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
    if (verify_result != 1) {
        if (verify_result < 0) {
            // 回执校验失败，下次请求时再次校验
            *status_code = 0;
            *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
            googleplay_verify_rsp_body_t *ack_body =
            (googleplay_verify_rsp_body_t *)ack_body_buf;
            ack_body->verify_result = 4;  // 回执校验失败
            ack_body->trans_id = req_body->trans_id;
        }
        else {
            // 回执无效
            ret = g_mysql_handler->execsql("UPDATE googleplay_log_table SET receipt_id = %d, trans_time = NOW(), deal_time = NOW(), status = %d WHERE id = %d;", receipt_dbid, RECEIPT_DEALED, pay_log_id);
            if (ret < 0) {
                ERROR_LOG("update googleplay pay log failed, error = %s",
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
            *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
            googleplay_verify_rsp_body_t *ack_body =
            (googleplay_verify_rsp_body_t *)ack_body_buf;
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
        
        if (strcasecmp(receipt_pdt_id, "g006") == 0) {
            golds_to_add = 60000;
            costs = 600;
        }
        else if (strcasecmp(receipt_pdt_id, "g012") == 0) {
            golds_to_add = 120000;
            costs = 1200;
        }
        else if (strcasecmp(receipt_pdt_id, "g030") == 0) {
            golds_to_add = 350000;
            costs = 3000;
        }
        else if (strcasecmp(receipt_pdt_id, "g050") == 0) {
            golds_to_add = 600000;
            costs = 5000;
        }
        else if (strcasecmp(receipt_pdt_id, "g128") == 0) {
            golds_to_add = 1600000;
            costs = 12800;
        }
        else if (strcasecmp(receipt_pdt_id, "g258") == 0) {
            golds_to_add = 3300000;
            costs = 25800;
        }
        else if (strcasecmp(receipt_pdt_id, "g648") == 0) {
            golds_to_add = 9000000;
            costs = 64800;
        }
        //钻石
        else if (strcasecmp(receipt_pdt_id, "d006") == 0) {
            diamonds_to_add = 60;
            costs = 600;
        }
        else if (strcasecmp(receipt_pdt_id, "d012") == 0) {
            diamonds_to_add = 120;
            costs = 1200;
        }
        else if (strcasecmp(receipt_pdt_id, "d030") == 0) {
            diamonds_to_add = 300;
            costs = 3000;
        }
        else if (strcasecmp(receipt_pdt_id, "d050") == 0) {
            diamonds_to_add = 500;
            costs = 5000;
        }
        else if (strcasecmp(receipt_pdt_id, "d128") == 0) {
            diamonds_to_add = 1280;
            costs = 12800;
        }
        else if (strcasecmp(receipt_pdt_id, "d258") == 0) {
            diamonds_to_add = 2580;
            costs = 25800;
        }
        else if (strcasecmp(receipt_pdt_id, "d648") == 0) {
            diamonds_to_add = 6480;
            costs = 64800;
        }
        //其他商品
        else if (strcasecmp(receipt_pdt_id, "l951") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 951;
        }
        else if (strcasecmp(receipt_pdt_id, "l952") == 0) {
            others_goods_to_add = 1;
            costs = 3000;
            item_id = 952;
        }
        else {
            // 未知的商品id
            ERROR_LOG("Unknown pdt id: %d, %d", user_id, receipt_pdt_id);
        }
        
        if (golds_to_add > 0 || diamonds_to_add > 0 || others_goods_to_add > 0)
        {
            ret = g_mysql_handler->execsql("UPDATE googleplay_log_table SET costs = %d, golds = %d, diamonds = %d, receipt_id = %d, trans_time = NOW(), status = %d WHERE id = %d;", costs, golds_to_add, diamonds_to_add, receipt_dbid, RECEIPT_VERIFIED, pay_log_id);
            if (ret < 0)
            {
                ERROR_LOG("update googleplay pay log failed, error = %s",
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
            
            ret = g_mysql_handler->execsql("UPDATE googleplay_log_table SET beansys_trans_id = %d, deal_time = NOW(), status = %d WHERE id = %d;", beansys_trans_id, RECEIPT_DEALED, pay_log_id);
            if (ret < 0)
            {
                ERROR_LOG("update googleplay pay log failed, error = %s",
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
                *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
                googleplay_verify_rsp_body_t *ack_body =
                (googleplay_verify_rsp_body_t *)ack_body_buf;
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
            ret = g_mysql_handler->execsql("UPDATE googleplay_log_table SET costs = %d, golds = %d, diamonds = %d, receipt_id = %d, trans_time = NOW(), deal_time = NOW(), status = %d WHERE id = %d;", costs, golds_to_add, diamonds_to_add, receipt_dbid, RECEIPT_DEALED, pay_log_id);
            if (ret < 0)
            {
                ERROR_LOG("update googleplay pay log failed, error = %s",
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
    *ack_body_len = sizeof(googleplay_verify_rsp_body_t);
    googleplay_verify_rsp_body_t *ack_body =
    (googleplay_verify_rsp_body_t *)ack_body_buf;
    ack_body->verify_result = 0;  // 处理成功
    ack_body->trans_id = req_body->trans_id;

    return 0;
}

