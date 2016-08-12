#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "mq_mgr.h"
#include "ini_file.h"
#include "db_iface.h"
#include "work.h"
#include "init.h"

char* prog_name;
//char* current_dir;

c_mq_mgr_t g_mq_mgr;
db_iface g_db_iface; // 数据库接口对象

unsigned int g_target_count = 0;					// 同步的目标数
inet_address_t g_target_address[MAX_TARGET_COUNT];	// 同步的目标地址（IP和端口）

char g_msg_tables[MAX_TARGET_COUNT][256];			// 每个同步目标对应在数据库中的表名

bool g_stop = false; // flag on whether current process should stop and exit

void show_usage ();
static int parse_args (int argc, char **argv);

int main(int argc, char* argv[])
{
	///parse command line arguments and get configuration file
	parse_args(argc, argv);
	const char* p_config_file = argv[1];

	///register signal handler
	register_main_sighandler();

	int status = 0 ;

	///system init
	system_init_elements_t init_elements;
	init_elements.p_mq_mgr = &g_mq_mgr;
	init_elements.p_target_count = &g_target_count;
	init_elements.p_target_address = g_target_address;
	init_elements.target_address_len = sizeof(g_target_address)/sizeof(inet_address_t);
	init_elements.p_db_iface = &g_db_iface;
	init_elements.p_msg_tables = g_msg_tables;

	status = system_init(p_config_file, &init_elements);
	if (status == 0)
	{
		boot_log(0, 0, "system init");
		INFO_LOG("system init success");
		g_db_iface.close_db_connect(); //关闭数据库连接
	}
	else
	{
		boot_log(-1, 0, "system init");
		INFO_LOG("system init failed");
		exit(-1);
	}

	///输出系统一些参数信息
	INFO_LOG("params: MAX_MSG_LEN=%d, MAX_TARGET_COUNT=%d, BATCH_RECEIVE_COUNT=%d, BATCH_QUERY_COUNT=%d", MAX_MSG_LEN, MAX_TARGET_COUNT, BATCH_RECEIVE_COUNT, BATCH_QUERY_COUNT);

	///set process to daemon
	status = daemon (1, 1);
	if (status == 0)
	{
		boot_log(0, 0, "switch to daemon mode");
	}
	else
	{
		boot_log(-1, 0, "switch to daemon mode");
		exit(-1);
	}


	///fork receive and send processes
	do
	{
		status = fork_receive_process();
		if (status == 0)
		{
			boot_log(0, 0, "fork receive process");
		}
		else
		{
			boot_log(-1, 0, "fork receive process");
			break;
		}

		status = fork_send_process();
		if (status == 0)
		{
			boot_log(0, 0, "fork %d send process", g_target_count);
		}
		else
		{
			boot_log(-1, 0, "fork %d send process", g_target_count);
			break;
		}
	} while (false);

	///check if forking children processes successful
	if (status == 0)
	{
		INFO_LOG("system start success");
	}
	else
	{
		kill(0, SIGTERM); // send TERM signal to the current process group
		wait_children();  // wait for all child processes to exit

		INFO_LOG("=====main process exit=====");
		exit(-1);
	}

	///monitor message queue status
	monitor_mq();

	///exit
	wait_children();  // wait for all children processes to exit
	INFO_LOG("=====main process exit=====");
	return 0;
}

void show_usage ()
{
	printf ("Usage: %s conf [option]\n", prog_name);
	exit (0);
}

static int parse_args (int argc, char **argv)
{
	prog_name = strdup(argv[0]);
//	current_dir = strdup ((char*)get_current_dir_name ());

	if (argc < 2 || !strcmp (argv[1], "--help") || !strcmp (argv[1], "-h"))
	{
		show_usage ();
	}

	BOOT_LOG (0, "parse parameters of command line");
}

