/*
 * File:   mq_mgr.h
 */

//-----------------------------------//
//-----------------------------------//

#ifndef _MQ_MGR_H
#define	_MQ_MGR_H

#include <mqueue.h>

#include "log.h"
#include "util.h"

//-----------------------------------//
//-----------------------------------//

class c_mq_mgr_t
{
public:
    c_mq_mgr_t();
    ~c_mq_mgr_t();

    int init(const char* p_mq_name, int mode, int msg_max, int msgsize_max);
    int send_msg(const void* p_data,int data_len,int msg_priority);
    int send_msg_timedout(const void* p_data,int data_len,int msg_priority,int seconds);
    int recv_msg(char* p_recv_buffer,int buffer_len,unsigned int* p_msg_priority);
    int recv_msg_timedout(char* p_recv_buffer,int buffer_len,unsigned int* p_msg_priority,int seconds);
    int get_attr(long* mq_flags,long* mq_maxmsg,long* mq_msgsize,long* mq_curmsgs);
    int uninit();

private:
    int m_inited;
    mqd_t m_mq_fd;
};

//-----------------------------------//
//-----------------------------------//

#endif

//-----------------------------------//
//-----------------------------------//

