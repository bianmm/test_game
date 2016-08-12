//-----------------------------------//
//-----------------------------------//

#include <unistd.h>
#include <string.h>

#include "db_connection_mgr.h"

//-----------------------------------//
//-----------------------------------//

c_db_connection_mgr_t::c_db_connection_mgr_t()
{
    memset(&m_db_connection,0,sizeof(m_db_connection));
    m_inited = 0;
}

//-----------------------------------//
//-----------------------------------//

c_db_connection_mgr_t::~c_db_connection_mgr_t()
{
    uninit();
}

//-----------------------------------//
//-----------------------------------//

int c_db_connection_mgr_t::init(char* sz_db_ip,char* sz_db_user,char* sz_db_pwd,char* sz_db_name,int db_port)
{
    //-----------------------
    //chekc init status
    if(m_inited)
    {
        return -1;
    }
    
    //-----------------------
    //check params
    if(NULL == sz_db_ip || NULL == sz_db_user || NULL == sz_db_pwd || NULL == sz_db_name)
    {
        return -1;
    }
    
    //-----------------------
    //init mysql library
    memset(&m_db_connection,0,sizeof(m_db_connection));
    mysql_init(&m_db_connection);
    
    //-----------------------
    //set auto reconnect (mysql server version must > 5.019)
    my_bool is_auto_reconnect = 1;
    if(mysql_options(&m_db_connection,MYSQL_OPT_RECONNECT,&is_auto_reconnect))
    {
        return -1;
    }
    
    //-----------------------
    //connect to mysql server
    if(!mysql_real_connect(&m_db_connection,sz_db_ip,sz_db_user,sz_db_pwd,sz_db_name,db_port,NULL,0))
    {
        return -1;
    }
    
    //-----------------------
    //init success
    m_inited = 1;
    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_db_connection_mgr_t::get_db_connection(uint32_t account_id,mysql_connection_t** pp_db_connection)
{
    //-----------------------
    //check init status
    if(!m_inited)
    {
        return -1;
    }
    
    //-----------------------
    //param check
    if(NULL == pp_db_connection)
    {
        return -1;
    }
    
    //-----------------------
    //check whether the connection is alive
    mysql_ping(&m_db_connection);
    
    //-----------------------
    //return
    *pp_db_connection = &m_db_connection;
    return 0;
}

//-----------------------------------//
//-----------------------------------//

int c_db_connection_mgr_t::uninit()
{
    //-----------------------
    //check init status
    if(!m_inited)
    {
        return -1;
    }
    
    //-----------------------
    //close db connection
    mysql_close(&m_db_connection);
    
    m_inited = 0;
    return 0;
}

//-----------------------------------//
//-----------------------------------//

//-----------------------------------//
//-----------------------------------//
