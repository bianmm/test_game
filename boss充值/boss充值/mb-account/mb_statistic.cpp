//-----------------------------------//
//-----------------------------------//

#include <time.h>
#include <string.h>
#include <map>
using namespace std;

#include "mb_statistic.h"
#include "msglog.h"
#include "benchapi.h"

//-----------------------------------//
//-----------------------------------//

//-----------------------------------//
//-----------------------------------//

#define TOTAL_MB_ADDED_STATISTIC_ID      0x01010101
#define TOTAL_MB_CONSUMED_STATISTIC_ID   0x01010201

extern map<uint32_t, uint32_t> g_channel_stat_map;

unsigned int c_mb_statistic_t::get_statistic_id(int channel_id)
{
    unsigned int ret = g_channel_stat_map[channel_id];
    TRACE_LOG("chnl_id %u got stat_id 0x%08X", channel_id, ret);
    return ret;
}

//-----------------------------------//
//-----------------------------------//

c_mb_statistic_t::c_mb_statistic_t()
{
    memset(m_sz_statistic_file_full_path,0,sizeof(m_sz_statistic_file_full_path));
    m_inited = 0;
}

//-----------------------------------//
//-----------------------------------//

c_mb_statistic_t::~c_mb_statistic_t()
{
    uninit();
}

//-----------------------------------//
//-----------------------------------//

int c_mb_statistic_t::init(const char* p_statistic_file_full_path)
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

int c_mb_statistic_t::add_mb(int account_id,int mb_num,int channel_id)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //total mb added statistic
    time_t current_time = time(NULL);
    msglog(m_sz_statistic_file_full_path,TOTAL_MB_ADDED_STATISTIC_ID,current_time,&mb_num,sizeof(mb_num));

    //-------------------
    //channel mb added statistic
    int statistic_id = get_statistic_id(channel_id);
    msglog(m_sz_statistic_file_full_path,statistic_id,current_time,&mb_num,sizeof(mb_num));

    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_mb_statistic_t::consume_mb(int account_id,int mb_num,int channel_id,int product_id,int product_count)
{
    //-------------------
    //check module status
    if(!m_inited)
    {
        return -1;
    }

    //-------------------
    //total mb consumed statistic
    time_t current_time = time(NULL);
    msglog(m_sz_statistic_file_full_path,TOTAL_MB_CONSUMED_STATISTIC_ID,current_time,&mb_num,sizeof(mb_num));

    //-------------------
    //channel mb consumed statistic
    int statistic_id = get_statistic_id(channel_id);
    msglog(m_sz_statistic_file_full_path,statistic_id,current_time,&mb_num,sizeof(mb_num));

    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_mb_statistic_t::uninit()
{
    memset(m_sz_statistic_file_full_path,0,sizeof(m_sz_statistic_file_full_path));
    m_inited = 0;
    return 0;
}

//-----------------------------------//
//-----------------------------------//
