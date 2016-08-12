/**
 * ==================================================================================
 * @file  	init.cpp
 * @brief
 * @author  landry
 *
 * compiler:	g++ 4.1.2
 * platform:	Linux
 *
 * copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 * ==================================================================================
 */
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "init.h"
#include "log.h"
#include "util.h"
#include "ini_file.h"
#include "work.h"
#include "shm_util.h"

/**
 * @return 0 on success, -1 on error
 */
int init_log(const char* p_config_file)
{
	if (p_config_file == NULL)
	{
		return -1;
	}

	char log_dir[256] = { 0 };
	int log_level = 0;
	int log_size = 0;
	char log_prefix[32] = { 0 };

	read_profile_string("log", "log_dir", log_dir, sizeof(log_dir),"", p_config_file);
	log_level = read_profile_int("log", "log_level", APP_TRACE, p_config_file);
	log_size = read_profile_int("log", "log_size", 200000000, p_config_file); // default 200 million bytes
	read_profile_string("log", "log_prefix", log_prefix, sizeof(log_prefix),"", p_config_file);

	int ret = log_init (log_dir, log_level, log_size, log_prefix);
	if (ret == -1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


/**
 * @return 0 on success, -1 on error
 */
int init_mq(const char* p_config_file, c_mq_mgr_t* p_mq_mgr)
{
	if (p_config_file == NULL || p_mq_mgr == NULL)
	{
		return -1;
	}

	char mq_name[128] = { 0 };
	int mode = S_IRUSR|S_IWUSR|S_IXUSR; // 0700
	int msg_max = 0;
	int msgsize_max = 0;

	read_profile_string("mq", "name", mq_name, sizeof(mq_name),"", p_config_file);
	msg_max = read_profile_int("mq", "msg_max", 10, p_config_file);
	msgsize_max = read_profile_int("mq", "msgsize_max", 8192, p_config_file);

    INFO_LOG("mq_name: %s, msg_max; %d, msgsize_max: %d", mq_name, msg_max, msgsize_max);
	int ret = p_mq_mgr->init(mq_name, mode, msg_max, msgsize_max);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}

/**
 * @return 0 on success, -1 on error
 */
int init_db(const char* p_config_file, unsigned int target_count, const inet_address_t* p_target_address, db_iface* p_db_iface, char (*p_msg_tables)[256])
{
	///检查参数
	if (!p_config_file || target_count==0 || !p_target_address || !p_db_iface || !p_msg_tables)
	{
		return -1;
	}

	///读取数据库配置
	char db_ip[16] = { 0 };
	unsigned db_port = 0;
	char db_user[64] = { 0 };
	char db_passwd[64] = { 0 };
	char db_name[64] = { 0 };

	read_profile_string("db", "db_ip", db_ip, sizeof(db_ip), "0.0.0.0", p_config_file);
	db_port = read_profile_int("db", "db_port", 3306, p_config_file);
	read_profile_string("db", "db_user", db_user, sizeof(db_user), "unknown", p_config_file);
	read_profile_string("db", "db_passwd", db_passwd, sizeof(db_passwd), "unknown", p_config_file);
	read_profile_string("db", "db_name", db_name, sizeof(db_name), "unknown", p_config_file);

	///设置数据库接口对象
	p_db_iface->set_connect_info(db_ip, db_port, db_user, db_passwd, db_name);
	if (p_db_iface->connect_db())
	{
		ERROR_LOG("connect db failed, db_ip=%s, db_port=%u, db_user=%s, db_passwd=%s, db_name=%s", db_ip, db_port, db_user, db_passwd, db_name);
		return -1;
	}

	///根据同步目标确定数据库消息表名并创建
	for(int i=0; i<(int)target_count; i++)
	{
		///确定每个同步目标对应的数据库消息表名,格式示例: message_192_168_1_14_22001_table
		sprintf(p_msg_tables[i], "message_%s_%u_table", p_target_address[i].ip , p_target_address[i].port);
		string_char_replace(p_msg_tables[i], '.', '_');

		///创建消息表
		if (p_db_iface->create_msg_table(p_msg_tables[i]))
		{
			ERROR_LOG("DB error! create message table [%s] failed", p_msg_tables[i]);
			return -1;
		}

		///查询表中记录数，确认DB是OK的
		if (p_db_iface->query_msg_count(p_msg_tables[i]) == -1)
		{
			ERROR_LOG("DB error! p_db_iface->query_msg_count(%s)", p_msg_tables[i]);
			return -1;
		}
	}

	return 0;
}

/**
 * @return 0 on success, -1 on error
 */
int read_target_address(const char* p_config_file, unsigned int* p_target_count, inet_address_t* p_target_address, unsigned int target_address_len)
{
	if (!p_config_file || !p_target_count || !p_target_address || target_address_len==0)
	{
		return -1;
	}

	//-------------------
	//
	int target_count = read_profile_int("target", "target_count", 0, p_config_file);
	if (target_count <= 0 || target_count>MAX_TARGET_COUNT)
	{
		ERROR_LOG("key [%s] in section [%s] in file [%s] invalid", "target_count", "target", p_config_file);
		return -1;
	}
	else
	{
		*p_target_count = target_count;
	}

	//todo: compare target_count with target_address_len

	char ip_key[16] = { 0 };
	char port_key[16] = { 0 };

	for (int i=0; i<(int)(*p_target_count); i++)
	{
		sprintf(ip_key, "target_ip%d", i+1);
		sprintf(port_key, "target_port%d", i+1);

		int r1 = read_profile_string("target", ip_key, p_target_address[i].ip, sizeof(p_target_address[i].ip),"", p_config_file);
		int r2 = read_profile_int("target", port_key, 0, p_config_file);

		if (r1 == 0)
		{
			ERROR_LOG("key [%s] in section [%s] in file [%s] invalid", ip_key, "target", p_config_file);
			return -1;
		}

		if (r2 <= 0)
		{
			ERROR_LOG("key [%s] in section [%s] in file [%s] invalid", port_key, "target", p_config_file);
			return -1;
		}
		else
		{
			p_target_address[i].port = (unsigned short)r2;
		}

		INFO_LOG("target system ip:%s,port:%d", p_target_address[i].ip, p_target_address[i].port);
	}

	return 0;
}

/**
 * @return 0 on success, -1 on error
 */
int system_init(const char* p_config_file, system_init_elements_t* p_init_elements)
{
	///检查参数
	if (p_config_file == NULL || p_init_elements == NULL)
	{
		return -1;
	}

	if (p_init_elements->p_mq_mgr == NULL
		|| p_init_elements->p_target_count == NULL
		|| p_init_elements->p_target_address == NULL
		|| p_init_elements->p_db_iface == NULL
		|| p_init_elements->p_msg_tables == NULL)
	{
		return -1;
	}

	///
	//int retval = 0; //return value
	int status = 0;

	do
	{
		///log init
		status = init_log(p_config_file);
		if (status == 0)
		{
			INFO_LOG("init log success");
		}
		else
		{
			break;
		}

		///message queue init
		status = init_mq(p_config_file, p_init_elements->p_mq_mgr);
		if (status == 0)
		{
			INFO_LOG("init_mq() success");
		}
		else
		{
			ERROR_LOG("init_mq() failed");
			break;
		}

		///read target system address config
		status = read_target_address(p_config_file, p_init_elements->p_target_count, p_init_elements->p_target_address, p_init_elements->target_address_len);
		if (status == 0)
		{
			INFO_LOG("read_target_address() success");
		}
		else
		{
			ERROR_LOG("read_target_address() failed");
			break;
		}

		///db init
		status = init_db(p_config_file, *(p_init_elements->p_target_count), p_init_elements->p_target_address, p_init_elements->p_db_iface, p_init_elements->p_msg_tables);
		if (status == 0)
		{
			INFO_LOG("init_db() success");
		}
		else
		{
			ERROR_LOG("init_db() failed");
			break;
		}
	} while(false);

	if (status != 0)
	{
		return -1;
	}

	return 0;
}
