#ifndef _PROTO_H
#define _PROTO_H

#include <stdint.h>

/// 错误码定义

#define SYS_ERR 1001                // 系统出错 一般是内存出错
#define DB_ERR 1002                 // 数据库出错
#define NET_ERR 1003				// 游戏服网络连接错误
#define CMDID_NODEFINE_ERR 1004     // 命令ID没有定义
#define PROTO_LEN_ERR 1005          // 协议长度不符合
#define CMD_PARAM_ERR 1115			// 参数错误
#define RECORD_NOT_FOUND 1116

const uint32_t MAX_SNDBUF_LEN = 1024 * 8;

enum {
	RECEIPT_NOT_VERIFIED = 0, RECEIPT_VERIFIED = 1, RECEIPT_DEALED = 2,
};

struct proto_header_t {
	uint32_t pkg_len;
	uint32_t seq_num;
	uint16_t cmd_id;
	uint32_t status_code;
	uint32_t user_id;
}__attribute__((packed));

//anypay查询
#define ANYPAY_QUERY_CMD_ID 0xA046
struct anypay_query_req_body_t {
    uint32_t preorder_id;
}__attribute__((packed));

struct anypay_query_rsp_body_t {
    uint32_t preorder_id;
    uint32_t user_id;
    uint8_t channel_id;
    uint8_t pay_channel_id;
    uint32_t costs;
    uint8_t product_id[64];
    uint8_t ud[64];
}__attribute__((packed));

//anypay预交易包体
#define ANYPAY_PREORDER_CMD_ID 0xA047
struct anypay_preorder_req_body_t {
    char product_id[64];
    uint32_t costs;
	char ud[64];
}__attribute__((packed));

struct anypay_preorder_rsp_body_t {
    char product_id[64];
    uint32_t costs;
	uint32_t preorder_id;
}__attribute__((packed));

//anypay支付通知包体
#define ANYPAY_NOTIFY_CMD_ID 0xA048
#define PP_PAY_NOTIFY_CMD_ID 0xA049
#define XY_PAY_NOTIFY_CMD_ID 0xA050
#define BAIDU_SIGNE_NOTIFY_CMD_ID 0xA051
#define CHONGCHONG_PAY_NOTIFY_CMD_ID 0xA052

struct anypay_notify_req_body_t {
	uint16_t transdata_len;
	uint8_t data[0];
}__attribute__((packed));

struct anypay_notify_rsp_body_t {
	uint8_t verify_result;
	uint32_t preorder_id;
}__attribute__((packed));

#define ANYPAY_NOTIFY_CLIENT_CMD_ID 0xA053 // 客户端通知的交易确认，不是第三方服务端通知的

struct anypay_notify_client_req_body_t {
    uint16_t channel_id;
    uint32_t preorder_id;
    uint16_t transdata_len;
    uint8_t data[0];
}__attribute__((packed));

struct anypay_notify_client_rsp_body_t {
    uint8_t verify_result;
    uint32_t preorder_id;
}__attribute__((packed));

#endif
