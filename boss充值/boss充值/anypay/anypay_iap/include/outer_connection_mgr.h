/**
 * =====================================================================================
 *       @file  outer_connection_mgr.h
 *      @brief  负责短连接通信的接口
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

#ifndef __OUTER_CONNECTION_MGR_H__
#define __OUTER_CONNECTION_MGR_H__
#include <sys/types.h>
class outer_connection_mgr_t
{
public:
    outer_connection_mgr_t();
    ~outer_connection_mgr_t();

    int init(const char*, unsigned short);
    int ping();
    int do_io(const char*, int, char*, int, int, size_t);
    int do_io_text(const char*, int, char*, int, int);
    int uninit();

private:
    bool  m_inited;

    char m_addr[20];
    int  m_port;
};

#endif // ! __OUTER_CONNECTION_MGR_H__

