//-----------------------------------//
//-----------------------------------//

#ifndef _MYSQL_RES_AUTO_PTR_H_MINGLIN_XUU_080722_
#define _MYSQL_RES_AUTO_PTR_H_MINGLIN_XUU_080722_

//-----------------------------------//
//-----------------------------------//

#include <mysql/mysql.h>

//-----------------------------------//
//-----------------------------------//

class c_mysql_res_auto_ptr_t
{
public:
    c_mysql_res_auto_ptr_t(MYSQL_RES* p_record_set);
    ~c_mysql_res_auto_ptr_t();
    
    int detach();
    int free();
    
private:
    MYSQL_RES* m_p_record_set;
};

//-----------------------------------//
//-----------------------------------//

#endif//_MYSQL_RES_AUTO_PTR_H_MINGLIN_XUU_080722_

//-----------------------------------//
//-----------------------------------//
