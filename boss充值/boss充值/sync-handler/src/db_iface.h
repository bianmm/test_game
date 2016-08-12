#ifndef _DB_IFACE_H
#define _DB_IFACE_H

#include <stdint.h>

#include "db_connection_mgr.h"
#include "log.h"

//todo:放到makefile中
#define MAX_MSG_LEN  8192  //消息最大长度，根据需求配置该值,需大于等于配置文件中mq的msgsize_max值

struct message_t
{
	uint32_t id;
	char msg_buf[MAX_MSG_LEN];
	uint16_t msg_buf_len;
};

class db_iface
{
public:
	db_iface();

	db_iface(const char* ip, uint16_t port, const char* user_name, const char* passwd, const char* db_name);

	~db_iface();

	int set_connect_info(const char* ip, uint16_t port, const char* user_name, const char* passwd, const char* db_name);

	/**
	 * @return  0 on success, -1 on error
	 */
	int connect_db();

	/**
	 * @return  0 on success, -1 on error
	 */
	int reconnect_db();

	/**
	 * @return  0 on success, -1 on error
	 */
	int close_db_connect();

	/**
	 * @brief 创建消息表
	 */
	int create_msg_table(const char* msg_table);

	int insert_message(const char* msg_table, const message_t* msgs, unsigned int msg_count);

	/**
	 * @param  max_count  一次查询的最大消息数
	 * @return return the count of messages selected on success, -1 on error, -2 on db busy
	 */
	int select_message(const char* msg_table, message_t* msgs, unsigned int max_count);

	/**
	 * @return 1 on success, 0 if message with the specified id msg_id not exists, -1 on error
	 */
	int delete_message(const char* msg_table, uint32_t msg_id);

	/**
	 * @brief 查询指定表中总记录数
	 */
	int query_msg_count(const char* msg_table);

private:
	c_db_connection_mgr_t db_conn_mgr;

	char db_ip[16];
	uint16_t db_port;
	char db_user[64];
	char db_passwd[64];
	char db_name[64]; //数据库名

	char sql[256]; //SQL语句buffer
	char insert_sql[MAX_MSG_LEN * 2 + 256]; //insert_message() 使用
	char escaped_string[MAX_MSG_LEN * 2 + 1]; //insert_message() 使用
};

#endif //_DB_IFACE_H
