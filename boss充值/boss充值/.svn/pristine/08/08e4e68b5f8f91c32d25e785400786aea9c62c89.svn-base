#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "db_iface.h"
#include "mysql_res_auto_ptr.h"
#include "my_exception.h"

db_iface::db_iface()
{
	///
	memset(this->db_ip, 0, sizeof(this->db_ip));
	this->db_port = 0;
	memset(this->db_user, 0, sizeof(this->db_user));
	memset(this->db_passwd, 0, sizeof(this->db_passwd));
	memset(this->db_name, 0, sizeof(this->db_name));

	///
	memset(this->sql, 0, sizeof(this->sql));
	memset(this->insert_sql, 0, sizeof(this->insert_sql));
	memset(this->escaped_string, 0, sizeof(this->escaped_string));
}

db_iface::db_iface(const char* ip, uint16_t port, const char* user_name, const char* passwd, const char* db_name)
{
	///
	memset(this->db_ip, 0, sizeof(this->db_ip));
	this->db_port = 0;
	memset(this->db_user, 0, sizeof(this->db_user));
	memset(this->db_passwd, 0, sizeof(this->db_passwd));
	memset(this->db_name, 0, sizeof(this->db_name));

	///
	strncpy(this->db_ip, ip, sizeof(this->db_ip) - 1);
	this->db_port = port;
	strncpy(this->db_user, user_name, sizeof(this->db_user) - 1);
	strncpy(this->db_passwd, passwd, sizeof(this->db_passwd) - 1);
	strncpy(this->db_name, db_name, sizeof(this->db_name) - 1);

	///
	memset(this->sql, 0, sizeof(this->sql));
	memset(this->insert_sql, 0, sizeof(this->insert_sql));
	memset(this->escaped_string, 0, sizeof(this->escaped_string));
}

db_iface::~db_iface()
{

}

int db_iface::set_connect_info(const char* ip, uint16_t port, const char* user_name, const char* passwd, const char* db_name)
{
	///
	memset(this->db_ip, 0, sizeof(this->db_ip));
	memset(this->db_user, 0, sizeof(this->db_user));
	memset(this->db_passwd, 0, sizeof(this->db_passwd));
	memset(this->db_name, 0, sizeof(this->db_name));

	///
	strncpy(this->db_ip, ip, sizeof(this->db_ip) - 1);
	this->db_port = port;
	strncpy(this->db_user, user_name, sizeof(this->db_user) - 1);
	strncpy(this->db_passwd, passwd, sizeof(this->db_passwd) - 1);
	strncpy(this->db_name, db_name, sizeof(this->db_name) - 1);

	return 0;
}

/**
 * @return  0 on success, -1 on error
 */
int db_iface::connect_db()
{
	if ((this->db_conn_mgr).init(this->db_ip, this->db_user, this->db_passwd, this->db_name, this->db_port))
	{
		return -1;
	}

	return 0;
}

/**
 * @brief 重新获得一个mysql连接
 *
 * @return  0 on success, -1 on error
 */
int db_iface::reconnect_db()
{
	///关闭当前已连接
	(this->db_conn_mgr).uninit();

	///获取新连接
	if ((this->db_conn_mgr).init(this->db_ip, this->db_user, this->db_passwd, this->db_name, this->db_port))
	{
		return -1;
	}

	return 0;
}

/**
 * @brief 关闭数据库连接
 *
 * @return  0 on success, -1 on error
 */
int db_iface::close_db_connect()
{
	///关闭当前已连接
	(this->db_conn_mgr).uninit();

	return 0;
}

int db_iface::create_msg_table(const char* p_msg_table)
{
	///
	if (p_msg_table == NULL)
	{
		return -1;
	}

	///
	mysql_connection_t* p_db_connection = NULL;
	db_conn_mgr.get_db_connection(0, &p_db_connection);
	assert(p_db_connection != NULL);

	///
	memset(this->sql, 0 , sizeof(this->sql));
	sprintf (this->sql, "CREATE TABLE IF NOT EXISTS %s (id INT UNSIGNED AUTO_INCREMENT, message BLOB NOT NULL, PRIMARY KEY(id)) ENGINE=InnoDB  CHARSET=utf8", p_msg_table);

	if(mysql_real_query(p_db_connection, this->sql, strlen(this->sql)))
	{
		ERROR_LOG("mysql_real_query() failed, [SQL:%s]", this->sql);
		return -1;
	}

	return 0;
}

/**
 * @todo:提供另外一个选择,用prepared statement
 * @todo:msg_count的值多大合适？msg_count太大事务太大?
 *
 * @return 0 on success, -1 on error
 */
int db_iface::insert_message(const char* p_msg_table, const message_t* p_msgs, unsigned int msg_count)
{
	///检查参数
	if (p_msg_table == NULL || p_msgs == NULL)
	{
		return -1;
	}

	if (msg_count == 0)
	{
		return 0;
	}

	///获取mysql连接
	mysql_connection_t* p_db_connection = NULL;
	db_conn_mgr.get_db_connection(0, &p_db_connection);
	assert(p_db_connection != NULL);

	///定义一些变量
	int sql_length = 0; //this->insert_sql 存储的SQL字符串 的长度
	int escaped_str_len = 0; //this->escaped_string 中存储的转义字符串的长度

    try
    {
		///start db transaction
		if (mysql_autocommit(p_db_connection, false))
		{
			ERROR_LOG("mysql_autocommit() failed!");
			throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
		}

		///将所有p_msgs插入到数据库表中
		for (int i = 0; i < (int)msg_count; i++)
		{
			///
			sql_length = 0;

			///将message二进制数据转义
			escaped_str_len = mysql_real_escape_string(p_db_connection, this->escaped_string, p_msgs[i].msg_buf, p_msgs[i].msg_buf_len);
			if (escaped_str_len < p_msgs[i].msg_buf_len)
			{
				ERROR_LOG("mysql_real_escape_string() error, escaped_str_len=%d, msg_buf_len=%u", escaped_str_len, p_msgs[i].msg_buf_len);
				throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
			}

			///构造SQL语句, sprintf format: INSERT INTO %s (message) values ('blob data')
			sql_length = sprintf(this->insert_sql, "INSERT INTO %s (message) values (\'", p_msg_table);

			memcpy(this->insert_sql + sql_length, this->escaped_string, escaped_str_len);
			sql_length += escaped_str_len;

			sql_length += sprintf(this->insert_sql + sql_length, "\')");

			///execute query
			if (mysql_real_query(p_db_connection, this->insert_sql, sql_length))
			{
				ERROR_LOG("mysql_real_query() failed!");
				throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
			}
		}

		///commit db transaction
		if (mysql_commit(p_db_connection))
		{
			ERROR_LOG("mysql_commit() failed!");
			throw c_my_exception_t(mysql_errno(p_db_connection), mysql_error(p_db_connection));
		}

		///restore db auto commit
		mysql_autocommit(p_db_connection, true);
    }
	catch (c_my_exception_t& exception)
	{
		///catch exception
		mysql_rollback(p_db_connection);
		mysql_autocommit(p_db_connection, true);

		ERROR_LOG("catch exception! errno=%d, errmsg=%s", exception.get_exception_code(), exception.get_exception_message());
		return -1;
	}

	return 0;
}

/**
 * @param  max_count  一次查询的最大消息数
 * @return return the count of messages selected on success, -1 on error
 */
int db_iface::select_message(const char* p_msg_table, message_t* p_msgs, unsigned int max_count)
{
	///check parameters
	if (p_msg_table == NULL || p_msgs == NULL)
	{
		return -1;
	}

	if (max_count == 0)
	{
		return 0;
	}

	///获取mysql连接
	mysql_connection_t* p_db_connection = NULL;
	db_conn_mgr.get_db_connection(0, &p_db_connection);
	assert(p_db_connection != NULL);

	///
	memset(this->sql, 0 , sizeof(this->sql));
	sprintf (this->sql, "SELECT id, message FROM %s ORDER BY id ASC LIMIT %u", p_msg_table, max_count); //最多查询 max_count 条记录

	if (mysql_real_query(p_db_connection, this->sql, strlen(this->sql)))
	{
		ERROR_LOG("mysql_real_query() failed, [SQL:%s]", this->sql);
		return -1;
	}

	mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
	c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set);

	if(NULL == p_record_set)
	{
		return -1;
	}

	int rows = 0; //查询结果集记录数
	unsigned long* lengths = NULL; //一行记录各个字段长度
	mysql_row_t data_row;

	while ((data_row = mysql_fetch_row(p_record_set)))
	{
		lengths = mysql_fetch_lengths(p_record_set);

		p_msgs[rows].id = atoi(data_row[0]);
		memcpy(p_msgs[rows].msg_buf, data_row[1], lengths[1]);
		p_msgs[rows].msg_buf_len = lengths[1];

		rows++;
	}

	return rows;
}


/**
 * @return 0 on success, -1 on error
 */
int db_iface::delete_message(const char* p_msg_table, uint32_t msg_id)
{
	///
	if (p_msg_table == NULL)
	{
		return -1;
	}

	///
	mysql_connection_t* p_db_connection = NULL;
	db_conn_mgr.get_db_connection(0, &p_db_connection);
	assert(p_db_connection != NULL);

	///
	memset(this->sql, 0 , sizeof(this->sql));
	sprintf (this->sql, "DELETE FROM %s WHERE id=%u LIMIT 1", p_msg_table, msg_id);

	if(mysql_real_query(p_db_connection, sql, strlen(sql)))
	{
		ERROR_LOG("mysql_real_query() failed, [SQL:%s]", this->sql);
		return -1;
	}

	if (mysql_affected_rows(p_db_connection) <= 0)
	{
		WARN_LOG("bug:message [id:%u] in table [%s] not exist!", msg_id, p_msg_table);
	}

	return 0;
}


/**
 * @brief 查询指定消息表中总记录数
 *
 * @return return the count of messages in the message table, -1 on error
 */
int db_iface::query_msg_count(const char* p_msg_table)
{
	///
	if (p_msg_table == NULL)
	{
		return -1;
	}

	///
	mysql_connection_t* p_db_connection = NULL;
	db_conn_mgr.get_db_connection(0, &p_db_connection);
	assert(p_db_connection != NULL);

	///
	memset(this->sql, 0 , sizeof(this->sql));
	sprintf (this->sql, "SELECT COUNT(*) FROM %s", p_msg_table);

	if(mysql_real_query(p_db_connection, this->sql, strlen(this->sql)))
	{
		ERROR_LOG("mysql_real_query() failed, [SQL:%s]", this->sql);
		return -1;
	}

	mysql_res_t* p_record_set = mysql_store_result(p_db_connection);
	c_mysql_res_auto_ptr_t res_auto_ptr(p_record_set);

	if(NULL == p_record_set)
	{
		return -1;
	}

	mysql_row_t data_row = mysql_fetch_row(p_record_set);
	if (data_row)
	{
		if (data_row[0])
		{
			return atoi(data_row[0]);
		}
	}

	return -1;
}
