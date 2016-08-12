/*
 * File:   seer_bean_stat.h
 * Author: xml
 *
 * Created on November 6, 2008, 11:03 AM
 */

//-----------------------------------//
//-----------------------------------//

#ifndef _SEER_BEAN_STAT_H_MINGLINXUU_20090618_
#define	_SEER_BEAN_STAT_H_MINGLINXUU_20090618_

//-----------------------------------//
//-----------------------------------//

class c_seer_bean_stat_t
{
public:
    c_seer_bean_stat_t();
    ~c_seer_bean_stat_t();

    int init(const char* p_statistic_file_full_path);
    int add_seer_bean(int account_id,int seer_bean_num, int channel_id);
    int consume_seer_bean(int account_id,int seer_bean_num,int channel_id,int product_id,int product_count);
    int uninit();

public:
    static unsigned int get_add_statistic_id(int channel_id);
    static unsigned int get_consume_statistic_id(int channel_id);

private:
    int m_inited;
    char m_sz_statistic_file_full_path[1024];
};

//-----------------------------------//
//-----------------------------------//

#endif//_SEER_BEAN_STAT_H_MINGLINXUU_20090618_

//-----------------------------------//
//-----------------------------------//
