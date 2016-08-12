#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "beansys_iface.h"
#include "benchapi.h"
#include "c_ip_restrictor.h"
#include "cmd_handler.h"
#include "i_mysql_iface.h"
#include "proto.h"

i_mysql_iface *g_mysql_handler = NULL;
char g_send_buf[4096];
c_ip_restrictor g_ip_restrictor;
beansys_iface_t g_beansys;
beansys_iface_t g_diamonds;

int g_beansys_chnl_id = 0;
char g_beansys_chnl_key[9] = { 0 };

int handle_init(int argc, char **argv, int pid_type) {
	if (pid_type == PROC_WORK) {
		char *conf_str = NULL;
		char mysql_host[256] = { 0 };
		uint16_t mysql_port = 0;
		char mysql_user[256] = { 0 };
		char mysql_passwd[256] = { 0 };
		char mysql_db[256] = { 0 };

		conf_str = config_get_strval("MYSQL_HOST");
		if (conf_str != NULL) {
			strcpy(mysql_host, conf_str);
			DEBUG_LOG("set mysql host: %s", mysql_host);
		} else {
			ERROR_LOG("can not get config 'MYSQL_HOST'");
			return -1;
		}

		mysql_port = config_get_intval("MYSQL_PORT", 0);
		DEBUG_LOG("set mysql port: %u", mysql_port);

		conf_str = config_get_strval("MYSQL_USER");
		if (conf_str != NULL) {
			strcpy(mysql_user, conf_str);
			DEBUG_LOG("set mysql user: %s", mysql_user);
		} else {
			ERROR_LOG("can not get config 'MYSQL_USER'");
			return -1;
		}

		conf_str = config_get_strval("MYSQL_PASSWD");
		if (conf_str != NULL) {
			strcpy(mysql_passwd, conf_str);
			DEBUG_LOG("set mysql passwd: %s", mysql_passwd);
		} else {
			ERROR_LOG("can not get config 'MYSQL_PASSWD'");
			return -1;
		}

		conf_str = config_get_strval("MYSQL_DB");
		if (conf_str != NULL) {
			strcpy(mysql_db, conf_str);
			DEBUG_LOG("set mysql db: %s", mysql_db);
		} else {
			ERROR_LOG("can not get config 'MYSQL_DB'");
			return -1;
		}

		int ret = 0;
		ret = create_mysql_iface_instance(&g_mysql_handler);
		if (ret != 0 || g_mysql_handler == NULL) {
			ERROR_LOG("create mysql instance failed");
			return -1;
		}

		ret = g_mysql_handler->init(mysql_host, mysql_port, mysql_db,
				mysql_user, mysql_passwd, "utf8");
		if (ret != 0) {
			ERROR_LOG("mysql init failed, error = %s",
					g_mysql_handler->get_last_errstr());
			return -1;
		}

		char* beansys_addr = config_get_strval("beansys_addr");
		if (beansys_addr == NULL) {
			DEBUG_LOG("NULL beansys addr");
			return -1;
		}
		short beansys_port = config_get_intval("beansys_port", 0);
		if (g_beansys.init(beansys_addr, beansys_port) != 0) {
			ERROR_LOG("Init beansys fail, ret: %d", ret);
			/// 连接失败时继续运行不退出
		}
        
        //初始化钻石充值服务的地址
        char* diamonds_addr = config_get_strval("diamonds_addr");
        if (diamonds_addr == NULL) {
            DEBUG_LOG("NULL diamonds addr");
            return -1;
        }
        short diamonds_port = config_get_intval("diamonds_port", 0);
        if (g_diamonds.init(diamonds_addr, diamonds_port) != 0) {
            ERROR_LOG("Init diamonds fail, ret: %d", ret);
            /// 连接失败时继续运行不退出
        }

        //渠道id 和 key 共用
		g_beansys_chnl_id = config_get_intval("beansys_chnl_id", 0);
		char* beansys_chnl_key = config_get_strval("beansys_chnl_key");
		if (beansys_chnl_key == NULL) {
			DEBUG_LOG("NULL beansys_chnl_key");
			return -1;
		}
		memcpy(g_beansys_chnl_key, beansys_chnl_key, 8);

		return 0;

	} else if (pid_type == PROC_MAIN) {
		const char *conf_str = NULL;
		int ret = 0;

		conf_str = config_get_strval("ALLOW_IPS");
		if (conf_str == NULL) {
			ERROR_LOG("can not get config 'ALLOW_IPS'");
			return -1;
		} else {
			boot_log(0, 0, "set allow ips = '%s'", conf_str);
		}

		ret = g_ip_restrictor.init(conf_str);
		if (ret != 0) {
			ERROR_LOG("ip restrictor init failed");
			return -1;
		}
	}

	return 0;
}

int handle_open(char **send_buf, int *send_len, const skinfo_t *sk) {
	in_addr in;
	const char *ip_str = NULL;
	int is_valid = 0;

	memset(&in, 0, sizeof(in));
	in.s_addr = sk->remote_ip;
	ip_str = inet_ntoa(in);

	is_valid = g_ip_restrictor.is_valid(ip_str);
	if (is_valid != 0) {
		ERROR_LOG("ip '%s' deny", ip_str);
		return -1;
	}

	return 0;
}

int handle_input(const char *recv_buf, int recv_len, const skinfo_t *sk) {
	if (recv_len < (int) sizeof(uint32_t)) {
		return 0;
	}

	uint32_t pkg_len = *((uint32_t *) recv_buf);

	if (recv_len < (int) pkg_len) {
		return 0;
	}

	if (pkg_len < sizeof(proto_header_t)) {
		ERROR_LOG("pke_len %u is too small", pkg_len);
		return -1;
	}

	if (pkg_len > MAX_SNDBUF_LEN) {
		ERROR_LOG("pkg_len %u is too large", pkg_len);
		return -1;
	}
	DEBUG_LOG("pkg_len");
	DEBUG_LOG("pkg_len is %u", pkg_len);
	return pkg_len;
}

int handle_process(const char *recv_buf, int recv_len, char **send_buf,
		int *send_len, const skinfo_t *sk) {
	DEBUG_LOG("handle_process");
	proto_header_t *req_header = (proto_header_t *) (recv_buf);
	if (recv_len != (int) req_header->pkg_len) {
		ERROR_LOG("recv_len and pkg_len are not equal");
		return -1;
	}
	proto_header_t *ack_header = (proto_header_t *) (g_send_buf);
	ack_header->pkg_len = sizeof(proto_header_t);
	ack_header->seq_num = req_header->seq_num;
	ack_header->cmd_id = req_header->cmd_id;
	ack_header->status_code = 0;
	ack_header->user_id = req_header->user_id;
	*send_buf = g_send_buf;
	*send_len = sizeof(proto_header_t);

	uint32_t user_id = req_header->user_id;
	const char *req_body_buf = recv_buf + sizeof(proto_header_t);
	int req_body_len = recv_len - sizeof(proto_header_t);
	char *rsp_body_buf = g_send_buf + sizeof(proto_header_t);
	int rsp_body_len = 0;
	uint32_t *ack_status_code = &ack_header->status_code;

	switch (req_header->cmd_id) {
	case IPAY_PREORDER_CMD_ID:
		ipay_preorder(user_id, req_body_buf, req_body_len, rsp_body_buf,
				&rsp_body_len, ack_status_code);
		break;
	case IPAY_NOTIFY_CMD_ID:
		ipay_notify(user_id, req_body_buf, req_body_len, rsp_body_buf,
				&rsp_body_len, ack_status_code);
		break;
	default:
		ack_header->status_code = CMDID_NODEFINE_ERR;
		return 0;
		break;
	}

	*send_len = ack_header->pkg_len = sizeof(proto_header_t) + rsp_body_len;

	return 0;
}

int handle_fini(int pid_type) {
	if (pid_type == PROC_WORK) {
		delete g_mysql_handler;
		g_mysql_handler = NULL;
	}

	return 0;
}
