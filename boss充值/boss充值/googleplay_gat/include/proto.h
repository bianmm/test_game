
#ifndef _PROTO_H
#define _PROTO_H

#include <stdint.h>

/// 错误码定义

#define SYS_ERR 1001                /// 系统出错 一般是内存出错
#define DB_ERR 1002                 /// 数据库出错
#define NET_ERR 1003
#define CMDID_NODEFINE_ERR 1004     /// 命令ID没有定义
#define PROTO_LEN_ERR 1005          /// 协议长度不符合
#define ENUM_OUT_OF_RANGE_ERR 1009  /// 枚举越界
#define KEY_EXISTED_ERR 1014        /// 在insert，KEY已存在
#define KEY_NOFIND_ERR  1015        ///在select update, delect 时，KEY不存在

#define USER_ID_NOFIND_ERR  1105    ///在select update, delete 时，米米号不存在'

#define CMD_PARAM_ERR 1115
#define RECEIPT_ALREADY_DEALED 1116
#define PREORDER_NOT_EXISTS 1117
#define RECEIPT_INVALID 1118
#define RECEIPT_VERIFY_FAIL 1119

enum
{
    RECEIPT_NOT_VERIFIED = 0,
    RECEIPT_VERIFIED = 1,
    RECEIPT_DEALED = 2,
};

struct proto_header_t
{
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed));

#define GOOGLEPLAY_PREORDER_CMD_ID 0xA048

struct googleplay_preorder_req_body_t
{
    uint32_t product_id;
    uint8_t ud[64];
} __attribute__((packed));

struct googleplay_preorder_ack_body_t
{
    uint32_t product_id;
    uint32_t trans_id;
} __attribute__((packed));

#define GOOGLEPLAY_VERIFY_CMD_ID 0xA049

struct googleplay_verify_req_body_t
{
    uint32_t trans_id;
    uint16_t purchase_data_len;
    uint8_t purchase_data[0];
    //uint16_t data_sig_len;
    //uint8_t data_sig[0];
} __attribute__((packed));

struct googleplay_verify_req_body_another_part_t
{
    uint16_t data_sig_len;
    uint8_t data_sig[0];
} __attribute__((packed));

struct googleplay_verify_rsp_body_t
{
    uint8_t verify_result;
    uint32_t trans_id;
} __attribute__((packed));

#endif
