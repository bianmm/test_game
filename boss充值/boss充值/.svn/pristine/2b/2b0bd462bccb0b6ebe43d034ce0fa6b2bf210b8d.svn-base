/**
 * =====================================================================================
 *       @file  beansys_iface.cpp
 *      @brief  金豆系统的接口
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  01/31/2010 12:41:06 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2009, TaoMee.Inc, ShangHai.
 *
 *     @author  jasonwang (王国栋) jasonwang@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include "../include/beansys_iface.h"
#include "../include/benchapi.h"
#include "../include/functions.h"

extern int g_beansys_chnl_id;
extern char g_beansys_chnl_key[9];

/**
 * @fn beansys_iface_t::add_seer_bean
 * @brief 增加金豆
 *
 * @param uint32_t bean_num 需要增加的金豆数量
 * @param uint32_t ext_trans_id 本次事务流水ID
 * @param int32_t* result 接口的返回值
 * @param uint64_t* trans_id 交易ID
 *
 * @return int 0 执行成功 -1 执行失败
 *
 * @note
 * @see 
 */
int beansys_iface_t::add_seer_bean(uint32_t user_id, uint32_t bean_num, uint32_t costs, uint32_t item_id, uint32_t ext_trans_id, uint32_t channel_id, int32_t* result, uint64_t* trans_id)
{
    /// 构造请求包头
    beansys_req_add_seer_bean_t mb;
    mb.h.pkg_len = sizeof(beansys_req_add_seer_bean_t);
    mb.h.cmd_id = MB_BEANSYS_ADD_SEER_BEAN_ID;
    mb.h.result = 0;
    mb.h.user_id = user_id;

    /// 构造请求包体
    mb.bean_num = bean_num;
    mb.costs = costs;
    mb.item_id = item_id;
    mb.ext_trans_id = ext_trans_id;
    mb.channel_id = channel_id;

    /// 构造哈希
    if (fill_chnlhash32(&(mb.vfy_h), g_beansys_chnl_id, g_beansys_chnl_key, reinterpret_cast<const char*>(&mb) + sizeof(beansys_pkg_header_t) +
             sizeof(chnlhash32_vfy_header_t), sizeof(beansys_req_add_seer_bean_t) - sizeof(beansys_pkg_header_t) - sizeof(chnlhash32_vfy_header_t)) != 0) {
        ERROR_LOG("Construct channel hash fail.");
        return -1;
    }

    /// 发送包体
    int ret = do_io((const char*)&mb, mb.h.pkg_len, m_rcv_buf, BEANSYS_MAX_PKG_LEN, BEANSYS_TIME_OUT_SECONDS, sizeof(int32_t));
    if ( ret < 0 ) {
        ERROR_LOG("I/O error to beansys svr ret: %d", ret);
        return -1;
    }

    /// 解析返回值
    beansys_ack_add_seer_bean_t* mb_ack = reinterpret_cast<beansys_ack_add_seer_bean_t*>(m_rcv_buf);

    if (result != NULL)
        *result = mb_ack->h.result;

    if (trans_id != NULL) {
        *trans_id = mb_ack->trans_id;
    }

    return 0;
}
