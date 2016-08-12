#ifndef _INIT_H
#define _INIT_H

#include "socket_util.h"
#include "mq_mgr.h"
#include "db_iface.h"

typedef struct
{
	c_mq_mgr_t* p_mq_mgr;
	unsigned int* p_target_count;

	inet_address_t* p_target_address;
	unsigned int target_address_len;

	db_iface* p_db_iface;
	char (*p_msg_tables)[256];
} system_init_elements_t;

/**
 * @return 0 on success, -1 on error
 */
int init_log(const char* p_config_file);

/**
 * @return 0 on success, -1 on error
 */
int init_mq(const char* p_config_file, c_mq_mgr_t* p_mq_mgr);

/**
 * @return 0 on success, -1 on error
 */
int init_db(const char* p_config_file, unsigned int target_count, const inet_address_t* p_target_address, db_iface* p_db_iface, char (*p_msg_tables)[256]);

/**
 * @return 0 on success, -1 on error
 */
int read_target_address(const char* p_config_file, unsigned int* p_target_count, inet_address_t* p_target_address, unsigned int target_address_len);

/**
 * @return 0 on success, -1 on error
 */
int system_init(const char* p_config_file, system_init_elements_t* p_init_elements);

#endif
