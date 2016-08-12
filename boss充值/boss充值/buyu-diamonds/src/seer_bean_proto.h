//-----------------------------------//
//-----------------------------------//

#ifndef _SEER_BEAN_PROTO_H_MINGLIN_XUU_20091208_
#define _SEER_BEAN_PROTO_H_MINGLIN_XUU_20091208_

//-----------------------------------//
//-----------------------------------//

#include <stdint.h>

//-----------------------------------//
//-----------------------------------//

#define PACK_S_OK                             0
#define PACK_S_FALSE                          1
#define PACK_E_UNKNOWN                        2
#define PACK_E_INVALID_PARAM                  3
#define PACK_E_OPERATION_FAILED               4
#define PACK_E_ACCOUNT_ALREADY_EXISTS         101
#define PACK_E_ACCOUNT_NOT_EXISTS             102
#define PACK_E_INVALID_PWD                    103
#define PACK_E_ACCOUNT_NOT_ACTIVATED          104
#define PACK_E_NOT_ENOUGH_SEER_BEAN           105
#define PACK_E_INVALID_PRODUCT_COUNT          106
#define PACK_E_BEYOND_MONTH_CONSUME_LIMIT     107
#define PACK_E_BEYOND_PER_CONSUME_LIMIT       108
#define PACK_E_NOT_ALLOW_TRANSFER             109
#define PACK_E_INVALID_SEQ                    110
#define PACK_E_INVALID_CHANNEL                111
#define PACK_E_TRANS_ALREADY_DEALED           112
#define PACK_E_ACCOUNT_LOCKED                 113
#define PACK_E_INVALID_TRANS_ID               114
#define PACK_E_TRANS_SMALLER_THAN_INIT        115

//-----------------------------------//
//-----------------------------------//

typedef struct {
    uint32_t package_len;
    uint32_t seq_num;
    uint16_t command_id;
    uint32_t status_code;
    uint32_t account_id;
    uint8_t  package_body[];
} __attribute__((packed)) seer_bean_proto_header_t;

//-----------------------------------//
//-----------------------------------//

typedef struct {
    uint16_t channel_id;
    uint8_t  sz_md5[32];
    uint8_t  data[];
} __attribute__((packed)) seer_bean_proto_verify_header_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_CREATE_ACCOUNT_CMD_ID    21001

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_ADD_CMD_ID   22001

typedef struct {
    seer_bean_proto_verify_header_t verify_header;
    uint32_t seer_bean_num;
    uint32_t costs;
    uint32_t item_id;      //购买的商品id
    uint32_t external_trans_id;
    uint32_t channel_id;
} __attribute__((packed)) seer_bean_proto_add_t;

typedef struct {
    uint64_t transaction_id;
} __attribute__((packed)) seer_bean_proto_add_ack_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_CONSUME_CMD_ID  22002

typedef struct {
    seer_bean_proto_verify_header_t verify_header;
    uint32_t dest_account_id;
    uint32_t product_id;
    uint16_t product_count;
    uint32_t seer_bean_num;
    uint32_t external_trans_id;
} __attribute__((packed)) seer_bean_proto_consume_t;

typedef struct {
    uint64_t transaction_id;
    uint32_t seer_bean_balance;
} __attribute__((packed)) seer_bean_proto_consume_ack_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_TRANSFER_CMD_ID  22003

typedef struct {
    seer_bean_proto_verify_header_t verify_header;
    uint32_t dest_account_id;
    uint32_t seer_bean_num;
} __attribute__((packed)) seer_bean_proto_transfer_t;

typedef struct {
    uint64_t transaction_id;
} __attribute__((packed)) seer_bean_proto_transfer_ack_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_QUERY_BALANCE_CMD_ID  23001

typedef struct {
    uint32_t seer_bean_balance;
    uint32_t account_status;
} __attribute__((packed)) seer_bean_proto_query_balance_ack_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_QUERY_TRANS_CMD_ID  23002

typedef struct {
    uint64_t transaction_id;
} __attribute__((packed)) seer_bean_proto_query_trans_t;

typedef struct {
    uint32_t account_id;
    uint32_t dest_account_id;
    uint32_t transaction_time;
    uint16_t channel_id;
    uint32_t external_trans_id;
    uint32_t product_id;
    uint16_t product_count;
    uint32_t seer_bean_num;
} __attribute__((packed)) seer_bean_proto_query_trans_ack_t;

//-----------------------------------//
//-----------------------------------//

#define SEER_BEAN_PROTO_QUERY_ACCOUNT_HISTORY_CMD_ID  23003

typedef struct {
    uint32_t start_time;
    uint32_t end_time;
    uint16_t start_row_index;
    uint16_t end_row_index;
} __attribute__((packed)) seer_bean_proto_query_account_history_t;

typedef struct {
    uint32_t transaction_time;
    uint16_t channel_id;
    uint32_t seer_bean_num;
    uint32_t seer_bean_balance;
    uint32_t product_id;
    uint16_t product_count;
} __attribute__((packed)) seer_bean_proto_account_history_row_t;

typedef struct {
    uint32_t total_row_count;
    uint32_t current_row_count;
    seer_bean_proto_account_history_row_t row_array[40];
} __attribute__((packed)) seer_bean_proto_query_account_history_ack_t;

#endif//_SEER_BEAN_PROTO_H_MINGLIN_XUU_20091208_

//-----------------------------------//
//-----------------------------------//
