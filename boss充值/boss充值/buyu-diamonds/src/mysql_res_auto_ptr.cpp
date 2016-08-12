//-----------------------------------//
//-----------------------------------//

#include "mysql_res_auto_ptr.h"
#include "my_errno.h"

//-----------------------------------//
//-----------------------------------//

#define NULL  0

//-----------------------------------//
//-----------------------------------//

c_mysql_res_auto_ptr_t::c_mysql_res_auto_ptr_t(MYSQL_RES* p_record_set)
{
    m_p_record_set = p_record_set;
}

//-----------------------------------//
//-----------------------------------//

c_mysql_res_auto_ptr_t::~c_mysql_res_auto_ptr_t()
{
    free();
}

//-----------------------------------//
//-----------------------------------//

int c_mysql_res_auto_ptr_t::detach()
{
    if(m_p_record_set != NULL)
    {
        m_p_record_set = NULL;
        
        return S_OK;
    }
    else
    {
        return E_LOGIC;
    }
}

//-----------------------------------//
//-----------------------------------//

int c_mysql_res_auto_ptr_t::free()
{
    if(m_p_record_set != NULL)
    {
        mysql_free_result(m_p_record_set);
        m_p_record_set = NULL;
        
        return S_OK;
    }
    else
    {
        return E_LOGIC;
    }
}

//-----------------------------------//
//-----------------------------------//
