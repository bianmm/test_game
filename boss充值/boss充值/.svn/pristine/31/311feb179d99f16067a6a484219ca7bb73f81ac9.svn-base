//-----------------------------------//
//-----------------------------------//

#include <time.h>
#include <string.h>
#include "seer_bean_stat.h"
#include "msglog.h"

//-----------------------------------//
//-----------------------------------//

//-----------------------------------//
//-----------------------------------//

#define TOTAL_SEER_BEAN_ADDED_STATISTIC_ID      0x01050800  //总充值金豆数
#define TOTAL_SEER_BEAN_CONSUMED_STATISTIC_ID   0x01050900  //总消费金豆数

//充值
unsigned int c_seer_bean_stat_t::get_add_statistic_id(int channel_id)
{
    if(channel_id == 1001) //米币购买
    {
        return 0x01050801;
    }
    else if(channel_id == 1002) //购买商品赠送
    {
        return 0x01050802;
    }
    else if(channel_id == 1003) //奖品系统增加
    {
        return 0x01050804;
    }
    else if(channel_id == 1099) //客服
    {
    	return 0x01050803;
    }
    else
    {
    	return 0;
    }
}

//消费
unsigned int c_seer_bean_stat_t::get_consume_statistic_id(int channel_id)
{
    if(channel_id == 2001) //购买商品
    {
        return 0x01050901;
    }
    else if(channel_id == 2099) //客服
    {
        return 0x01050902;
    }
    else
    {
        return 0;
    }
}


//-----------------------------------//
//-----------------------------------//

c_seer_bean_stat_t::c_seer_bean_stat_t()
{
    memset(m_sz_statistic_file_full_path,0,sizeof(m_sz_statistic_file_full_path));
    m_inited = 0;
}

//-----------------------------------//
//-----------------------------------//

c_seer_bean_stat_t::~c_seer_bean_stat_t()
{
    uninit();
}

//-----------------------------------//
//-----------------------------------//

int c_seer_bean_stat_t::init(const char* p_statistic_file_full_path)
{
    //-------------------
    //check module status
    if(m_inited)
    {
        return -1;
    }

    //-------------------
    //check params
    if(NULL == p_statistic_file_full_path || strlen(p_statistic_file_full_path) <= 0)
    {
        return -1;
    }

    if(strlen(p_statistic_file_full_path) >= sizeof(m_sz_statistic_file_full_path))
    {
        return -1;
    }

    //-------------------
    //copy params
    strcpy(m_sz_statistic_file_full_path,p_statistic_file_full_path);
    m_inited = 1;

    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_seer_bean_stat_t::add_seer_bean(int account_id,int seer_bean_num, int channel_id)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //total bean added statistic
    time_t current_time = time(NULL);
    msglog(m_sz_statistic_file_full_path,TOTAL_SEER_BEAN_ADDED_STATISTIC_ID,current_time,&seer_bean_num,sizeof(seer_bean_num));

    //-------------------
    //channel bean added statistic
    int statistic_id = get_add_statistic_id(channel_id);
    msglog(m_sz_statistic_file_full_path,statistic_id,current_time,&seer_bean_num,sizeof(seer_bean_num));

    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_seer_bean_stat_t::consume_seer_bean(int account_id,int seer_bean_num,int channel_id,int product_id,int product_count)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //total bean consumed statistic
    time_t current_time = time(NULL);
    msglog(m_sz_statistic_file_full_path,TOTAL_SEER_BEAN_CONSUMED_STATISTIC_ID,current_time,&seer_bean_num,sizeof(seer_bean_num));

    //-------------------
    //channel bean consumed statistic
    int statistic_id = get_consume_statistic_id(channel_id);
    msglog(m_sz_statistic_file_full_path,statistic_id,current_time,&seer_bean_num,sizeof(seer_bean_num));

    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_seer_bean_stat_t::uninit()
{
    memset(m_sz_statistic_file_full_path,0,sizeof(m_sz_statistic_file_full_path));
    m_inited = 0;
    return 0;
}

//-----------------------------------//
//-----------------------------------//
