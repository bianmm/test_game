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

//爱贝预交易包体
#define IPAY_PREORDER_CMD_ID 0xA043
struct ipay_preorder_req_body_t {
	uint32_t zoneid;
	uint32_t price;
	uint32_t storeid;
	uint8_t ud[64];
}__attribute__((packed));

struct ipay_preorder_rsp_body_t {
	uint32_t zoneid;
	uint32_t price;
	uint32_t storeid;
	uint32_t trans_id;
}__attribute__((packed));

//爱贝支付通知包体
#define IPAY_NOTIFY_CMD_ID 0xA044
struct ipay_notify_req_body_t {
	uint16_t sign_len;
	uint16_t transdata_len;
	uint8_t data[0];
}__attribute__((packed));

struct ipay_notify_rsp_body_t {
	uint8_t verify_result;
	uint32_t trans_id;
}__attribute__((packed));

#endif
