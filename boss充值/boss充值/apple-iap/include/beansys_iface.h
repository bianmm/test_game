/**
 * =====================================================================================
 *       @file  beansys_iface.h
 *      @brief  金豆系统的接口
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  01/31/2010 12:41:05 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2009, TaoMee.Inc, ShangHai.
 *
 *     @author  jasonwang (王国栋) jasonwang@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */

#ifndef __BEANSYS_IFACE_H__
#define __BEANSYS_IFACE_H__

#include <stdint.h>

#include "outer_connection_mgr.h"
#include "functions.h"

/// 金豆系统的协议版本
#define BEANSYS_PROTOCOL_VERSION 1

/// 调用接口的超时时间
#define BEANSYS_TIME_OUT_SECONDS 8

#define BEANSYS_MAX_PKG_LEN (1024 * 4)

/// 金豆系统的返回状态码
#define BEANSYS_S_OK 0    // 操作成功
#define BEANSYS_S_FALSE 1    // 数据已经处于所要求的状态
#define BEANSYS_E_UNKNOWN_ERROR 2    // 未知错误
#define BEANSYS_E_INVALID_PARAM 3    // 数据包参数错误
#define BEANSYS_E_OPERATION_FAILED 4    // 操作失败
#define BEANSYS_E_ACCOUNT_ALREADY_EXIST 101    // 金豆账户已经存在,不能创建
#define BEANSYS_E_ACCOUNT_NOT_EXIST 102    // 金豆账户不存在
#define BEANSYS_E_INVALID_PWD 103    // 密码错误
#define BEANSYS_E_ACCOUNT_NOT_ACTIVATED 104    // 金豆账户还未激活
#define BEANSYS_E_NOT_ENOUGH_BEAN 105    // 金豆账号余额不足
#define BEANSYS_E_INVALID_PRODUCT_COUNT 106    // 购买产品数量错误
#define BEANSYS_E_BEYOND_MONTH_CONSUME_LIMIT 107    // 超过每月消费上限
#define BEANSYS_E_BEYOND_PER_CONSUME_LIMIT 108    // 超过单笔消费上限
#define BEANSYS_E_NOT_ALLOW_TRANSFER 109    // 帐户不允许转帐
#define BEANSYS_E_INVALID_SEQ 110    // 数据包序列号错误
#define BEANSYS_E_INVALID_CHANNEL 111    // 渠道不存在或接口验证错误
#define BEANSYS_E_TRANS_ALREADY_DEALED 112    // 充值或消费事务已经处理(重复包)
#define BEANSYS_E_ACCOUNT_LOCKED 113    // 米币抵用券账户已经被锁定
#define BEANSYS_E_INVALID_TRANS_ID 114    // 事务ID错误或不存在

/// 金豆系统命令包头
typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t result;
    uint32_t user_id;
} __attribute__((packed)) beansys_pkg_header_t;

/// 金豆系统的命令列表

/// 增加金豆命令 ID
#define MB_BEANSYS_ADD_SEER_BEAN_ID 22001

/// 增加金豆命令包体
typedef struct {
    beansys_pkg_header_t h;
    chnlhash32_vfy_header_t vfy_h;
    uint32_t bean_num;  /// 需要增加的金豆数量
    uint32_t costs;     //用户此次话费的人民币
    uint32_t item_id;     //用户此次购买的商品id
    uint32_t ext_trans_id;  /// 本次事务流水ID
    uint32_t channel_id;
} __attribute__((packed)) beansys_req_add_seer_bean_t;

/// 增加金豆命令返回包体
typedef struct {
    beansys_pkg_header_t h;
    uint64_t trans_id;  /// 交易ID
} __attribute__((packed)) beansys_ack_add_seer_bean_t;

/**
 * @class beansys_iface_t
 * @brief 金豆系统的接口
 *
 * @note 接口没有进行多线程互斥，使用时不允许多个线程同时调用接口
 * @see 
 */
class beansys_iface_t : public outer_connection_mgr_t
{
public:

    int add_seer_bean(uint32_t user_id, uint32_t bean_num, uint32_t costs, uint32_t item_id, uint32_t ext_trans_id, uint32_t channel_id, int32_t* result = NULL, uint64_t* trans_id = NULL);

private:
    char m_rcv_buf[BEANSYS_MAX_PKG_LEN];
};

#endif // !__BEANSYS_IFACE_H__

