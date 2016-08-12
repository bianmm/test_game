//-----------------------------------//
//-----------------------------------//

#ifndef _DB_CONNECTION_MGR_H_MINGLIN_XUU_20080822_
#define _DB_CONNECTION_MGR_H_MINGLIN_XUU_20080822_

//-----------------------------------//
//-----------------------------------//

#include <stdint.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

//-----------------------------------//
//-----------------------------------//

typedef MYSQL mysql_connection_t;
typedef MYSQL_RES mysql_res_t;
typedef MYSQL_ROW mysql_row_t;

//-----------------------------------//
//-----------------------------------//

class c_db_connection_mgr_t
{
public:
    c_db_connection_mgr_t();
    ~c_db_connection_mgr_t();
    
    int init(char* sz_db_ip,char* sz_db_user,char* sz_db_pwd,char* sz_db_name,int db_port);
    int get_db_connection(uint32_t account_id,mysql_connection_t** pp_db_connection);
    int uninit();
    
private:
    int m_inited;
    mysql_connection_t m_db_connection;
};

//-----------------------------------//
//-----------------------------------//

#endif//_DB_CONNECTION_MGR_H_MINGLIN_XUU_20080822_

//-----------------------------------//
//-----------------------------------//
