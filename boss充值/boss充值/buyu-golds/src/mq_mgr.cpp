//-----------------------------------//
//-----------------------------------//

#include <time.h>
#include <string.h>
#include <errno.h>

#include "mq_mgr.h"
#include "util.h"
#include "benchapi.h"
//-----------------------------------//
//-----------------------------------//

c_mq_mgr_t::c_mq_mgr_t()
{
    m_mq_fd = -1;
    m_inited = 0;
}

//-----------------------------------//
//-----------------------------------//

c_mq_mgr_t::~c_mq_mgr_t()
{
    uninit();
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::init(const char* p_mq_name,int mode, int msg_max, int msgsize_max)
{
    //-------------------
    //check module status
    if(m_inited)
    {
        ERROR_LOG("have inited");
        return -1;
    }

    //-------------------
    //check params
    if(NULL == p_mq_name || strlen(p_mq_name) <= 0 || msg_max < 0 || msgsize_max < 0 )
    {
        ERROR_LOG("invalid param");
        return -1;
    }

    //-------------------
    //open mq,create it if not exists
	struct mq_attr attriute;
	memset(&attriute, 0, sizeof(attriute));
	attriute.mq_maxmsg = msg_max;
	attriute.mq_msgsize = msgsize_max;

    m_mq_fd = mq_open(p_mq_name,O_RDWR|O_CREAT,mode, &attriute);
    if(m_mq_fd < 0)
    {
        ERROR_LOG("mq_open failed, name: %s, msg_max: %d, msgsize_max: %d, errno: %d, desc: %s", p_mq_name, msg_max, msgsize_max, errno, strerror(errno));
        return -1;
    }

    //-------------------
    //init successfully
    m_inited = 1;
    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::send_msg(const void* p_data,int data_len,int msg_priority)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //param check
    if(NULL == p_data || data_len < 0)
    {
        return -1;
    }

    //-------------------
    //send msg to mq
    if(mq_send(m_mq_fd,(char*)p_data,data_len,msg_priority))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::send_msg_timedout(const void* p_data,int data_len,int msg_priority,int seconds)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //param check
    if(NULL == p_data || data_len < 0 || seconds < 0)
    {
        return -1;
    }

    //-------------------
    //send msg with timedout
    timespec time_interval;
    memset(&time_interval,0,sizeof(time_interval));
    time_interval.tv_sec = time(NULL) + seconds;

    if(mq_timedsend(m_mq_fd,(char*)p_data,data_len,msg_priority,&time_interval))
    {
    	print_in_hex((char*)p_data, data_len);

        return -1;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::recv_msg(char* p_recv_buffer,int buffer_len,unsigned int* p_msg_priority)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //check params
    if(NULL == p_recv_buffer || buffer_len < 1)
    {
        return -1;
    }

    return mq_receive(m_mq_fd,p_recv_buffer,buffer_len,p_msg_priority);
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::recv_msg_timedout(char* p_recv_buffer,int buffer_len,unsigned int* p_msg_priority,int seconds)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //check params
    if(NULL == p_recv_buffer || buffer_len < 1 || seconds < 0)
    {
        return -1;
    }

    //-------------------
    //recv msg with timedout
    timespec time_interval;
    memset(&time_interval,0,sizeof(time_interval));
    time_interval.tv_sec = time(NULL) + seconds;

    return mq_timedreceive(m_mq_fd,p_recv_buffer,buffer_len,p_msg_priority,&time_interval);
}

//-----------------------------------//
//-----------------------------------//

int c_mq_mgr_t::uninit()
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //close mq
    mq_close(m_mq_fd);
    m_mq_fd = -1;
    m_inited = 0;

    return 0;
}

//-----------------------------------//
//-----------------------------------//
