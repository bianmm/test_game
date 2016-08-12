//-----------------------------------//
//-----------------------------------//

#include <string.h>
#include "my_exception.h"
#include "my_errno.h"

//-----------------------------------//
//-----------------------------------//

int min(int left,int right)
{
    if(left <= right)
    {
        return left;
    }
    else
    {
        return right;
    }
}

//-----------------------------------//
//-----------------------------------//

c_my_exception_t::c_my_exception_t()
{
    m_exception_code = 0;
    memset(m_sz_exception_message,0,sizeof(m_sz_exception_message));
}

//-----------------------------------//
//-----------------------------------//

c_my_exception_t::c_my_exception_t(int exception_code,const char* sz_exception_message)
{
    m_exception_code = exception_code;
    memset(m_sz_exception_message,0,sizeof(m_sz_exception_message));
    
    if(sz_exception_message != NULL)
    {
        strncpy(m_sz_exception_message,sz_exception_message,min(strlen(sz_exception_message),sizeof(m_sz_exception_message) - 1));
    }
}

//-----------------------------------//
//-----------------------------------//

c_my_exception_t::~c_my_exception_t()
{
}

//-----------------------------------//
//-----------------------------------//

int c_my_exception_t::get_exception_code()
{
    return m_exception_code;
}

//-----------------------------------//
//-----------------------------------//

int c_my_exception_t::get_exception_message(char* p_recv_buffer,int buffer_len)
{
    //----------------------
    //param check
    if(NULL == p_recv_buffer || buffer_len < 1)
    {
        return E_INVALIDARG;
    }
    
    memset(p_recv_buffer,0,buffer_len);
    strncpy(p_recv_buffer,m_sz_exception_message,min(buffer_len - 1,strlen(m_sz_exception_message)));
    return S_OK;
}

//-----------------------------------//
//-----------------------------------//

const char* c_my_exception_t::get_exception_message()
{
    return m_sz_exception_message;
}

//-----------------------------------//
//-----------------------------------//
