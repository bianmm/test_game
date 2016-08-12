/* 
 * File:   mb_statistic.h
 * Author: xml
 *
 * Created on November 6, 2008, 11:03 AM
 */

//-----------------------------------//
//-----------------------------------//

#ifndef _MB_STATISTIC_H_MINGLINXUU_20081106_
#define	_MB_STATISTIC_H_MINGLINXUU_20081106_

//-----------------------------------//
//-----------------------------------//

class c_mb_statistic_t
{
public:
    c_mb_statistic_t();
    ~c_mb_statistic_t();
    
    int init(const char* p_statistic_file_full_path);
    int add_mb(int account_id,int mb_num,int channel_id);
    int consume_mb(int account_id,int mb_num,int channel_id,int product_id,int product_count);
    int uninit();
    
public:
    static unsigned int get_statistic_id(int channel_id);
    
private:
    int m_inited;
    char m_sz_statistic_file_full_path[1024];
};

//-----------------------------------//
//-----------------------------------//

#endif//_MB_STATISTIC_H_MINGLINXUU_20081106_

//-----------------------------------//
//-----------------------------------//
