#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef DEBUG
#include <sys/time.h>
#endif

#include "work.h"
#include "shm_util.h"

extern bool g_stop;
extern c_mq_mgr_t g_mq_mgr;

extern int g_shm_id;
extern void* g_shm_address;

extern db_iface g_db_iface;
extern char g_msg_tables[MAX_TARGET_COUNT][256];

extern unsigned int g_target_count;
extern inet_address_t g_target_address[MAX_TARGET_COUNT];

void set_stop_flag(int signo)
{
	g_stop = true;
}

void wait_child(int signo)
{
	int status = 0;
	int pid = waitpid(-1, &status, WNOHANG);
	INFO_LOG("wait_child: pid:%d", pid);
}

int wait_children()
{
	int status = 0;
	int pid = 0;

	while (true)
	{
		pid = waitpid(-1, &status, 0);
		if (pid > 0)
		{
			INFO_LOG("wait_children: pid:%d", pid);
		}
		else
		{
			break;
		}
	}

	return 0;
}

int register_main_sighandler()
{
	struct sigaction sig_action;
	memset(&sig_action, 0, sizeof(sig_action));

	sig_action.sa_handler = SIG_IGN;
	//sig_action.sa_mask;
	//sig_action.sa_flags;

	//-------------------
	//ignore these signals
	sigaction(SIGHUP, &sig_action , NULL);
	sigaction(SIGPIPE, &sig_action , NULL);
	sigaction(SIGALRM, &sig_action , NULL);
	sigaction(SIGINT, &sig_action , NULL);
	sigaction(SIGUSR1, &sig_action , NULL);
	sigaction(SIGUSR2, &sig_action , NULL);
	//sigaction(SIGPOLL, &sig_action , NULL);
	sigaction(SIGPROF, &sig_action , NULL);
	sigaction(SIGVTALRM, &sig_action , NULL);

	//-------------------
	//catch these signals
	sig_action.sa_handler = set_stop_flag;
	sigaction(SIGTERM, &sig_action, NULL);

	sig_action.sa_handler = wait_child;
	sigaction(SIGCHLD, &sig_action, NULL);

	return 0;
}


/**
 *
 */
int register_receive_sighandler()
{
	struct sigaction sig_action;
	memset(&sig_action, 0, sizeof(sig_action));

	sig_action.sa_handler = SIG_IGN;
	//sig_action.sa_mask;
	//sig_action.sa_flags;

	//-------------------
	//ignore these signals
	sigaction(SIGHUP, &sig_action , NULL);
	sigaction(SIGPIPE, &sig_action , NULL);
	sigaction(SIGALRM, &sig_action , NULL);
	sigaction(SIGINT, &sig_action , NULL);
	sigaction(SIGUSR1, &sig_action , NULL);
	sigaction(SIGUSR2, &sig_action , NULL);
	//sigaction(SIGPOLL, &sig_action , NULL);
	sigaction(SIGPROF, &sig_action , NULL);
	sigaction(SIGVTALRM, &sig_action , NULL);

	//-------------------
	//catch these signals
	sig_action.sa_handler = set_stop_flag;
	sigaction(SIGTERM, &sig_action, NULL);

	return 0;
}

int register_send_sighandler()
{
	return register_receive_sighandler();
}

/**
 * @brief 从消息队列中读取消息，然后分别存入数据库中各消息表中
 *
 * @return  0 on success, -1 on error
 */
int receive_msgs()
{
	int status = 0;

	///
	static message_t msgs[BATCH_RECEIVE_COUNT];
	memset(&msgs, 0, sizeof(msgs));

	unsigned int cur_received_count = 0;

	while(!g_stop)
	{
		///receive a message from message queue
		status = g_mq_mgr.recv_msg_timedout(msgs[cur_received_count].msg_buf, sizeof(msgs[cur_received_count].msg_buf), NULL, 1);
		if (status > 0)
		{
			msgs[cur_received_count].msg_buf_len = status;
			cur_received_count++;
			if (cur_received_count < BATCH_RECEIVE_COUNT)
			{
				continue;
			}
		}
		else
		{
			if (errno == ETIMEDOUT)
			{
				//timeout
				if (cur_received_count == 0)
				{
					continue;
				}
			}
			else
			{
				//error
				ERROR_LOG("receive message from mq failed, [errno:%d][errstr:%s]", errno, strerror(errno));
				sleep(1);
				continue;
			}
		}

		///已经读到了 BATCH_RECEIVE_COUNT 条消息 或 读到了一些消息且再读mq时超时, 将消息存入数据库
		for (int i = 0; i < (int)g_target_count; i++)
		{
			///插入消息直到成功，如果一直不成功，则消息队列会被塞满
			while (!g_stop)
			{
#ifdef DEBUG
				struct timeval begin_time;
				gettimeofday(&begin_time, NULL);
				DEBUG_LOG("before g_db_iface.insert_message(), cur_received_count=%d, sec=%d, micro=%d", cur_received_count, begin_time.tv_sec, begin_time.tv_usec);
#endif
				status = g_db_iface.insert_message(g_msg_tables[i], msgs, cur_received_count);
				if (status == 0)
				{
					//success
#ifdef DEBUG
				struct timeval end_time;
				gettimeofday(&end_time, NULL);
				DEBUG_LOG("after g_db_iface.insert_message(), cur_received_count=%d, sec=%d, micro=%d", cur_received_count, end_time.tv_sec, end_time.tv_usec);
#endif
					break;
				}
				else
				{
					//error,这是很严重的问题，DB操作竟然失败！！！
					CRIT_LOG("DB error! g_db_iface.insert_message() failed, msg_table=%s, cur_received_count=%u", g_msg_tables[i], cur_received_count);
					sleep(1);
				}
			}
		}//for end

		cur_received_count = 0;
	}//while end

	///clear work
	g_mq_mgr.uninit();

	return 0;
}

int send_msgs(int index)
{
	///
	bool is_sock_open = false; // flag indicates if it's needed to reopen socket connection
	int sockfd = 0; // socket file descriptor
	static message_t msgs[BATCH_QUERY_COUNT];
	int status = 0;

	///loop to get message from DB and send it to target system
	while (!g_stop)
	{
		///open socket connection
		if (!is_sock_open)
		{
			close_socket(sockfd); // close socket if the previous send_one_message() failed.
			sockfd = open_socket(g_target_address[index].ip, g_target_address[index].port);
			if (sockfd == -1)
			{
				ERROR_LOG("connect %s:%d failed", g_target_address[index].ip, g_target_address[index].port);
				sleep(1);
				continue;
			}

			is_sock_open = true;
		}

		///get messages from DB
		memset(&msgs, 0, sizeof(msgs));
		int msg_count = g_db_iface.select_message(g_msg_tables[index], msgs, BATCH_QUERY_COUNT);
		if (msg_count > 0)
		{
			//success
		}
		else if(msg_count == 0)
		{
			//no row
			millisleep(500);
			continue;
		}
		else
		{
			//error
			ERROR_LOG("g_db_iface.select_message() failed! g_msg_tables[index]=%s", g_msg_tables[index]);
			millisleep(500);
			continue;
		}

		///send the messages to target systems
		for (int i=0; !g_stop && i<msg_count; i++)
		{
			///send one message to target systems
#ifdef DEBUG
			struct timeval begin_time;
			gettimeofday(&begin_time, NULL);
			DEBUG_LOG("before send_one_message(), i=%d, sec=%d, micro=%d", i, begin_time.tv_sec, begin_time.tv_usec);
#endif

			status = send_one_message(sockfd, g_target_address[index], &msgs[i]);
			if (status == -1)
			{
				//failure,retry
				is_sock_open = false;
				millisleep(500); //避免由于目标系统持续连接不上导致该进程的CPU usage过高
				break;
			}

#ifdef DEBUG
			struct timeval end_time;
			gettimeofday(&end_time, NULL);
			DEBUG_LOG("after send_one_message(), i=%d, sec=%d, micro=%d", i, end_time.tv_sec, end_time.tv_usec);
#endif

			///remove the message from DB
			while(!g_stop)
			{
				status = g_db_iface.delete_message(g_msg_tables[index], msgs[i].id);
				if (status == 0)
				{
					//success
					break;
				}
				else
				{
					//error
					ERROR_LOG("send_msgs: delete the message from DB error!, g_msg_tables[index]=%s, id=%d", g_msg_tables[index], msgs[i].id);
					millisleep(500); //避免由于DB操作不成功导致该进程的CPU usage过高
					continue;
				}
			} //while
		} //for
	}//while

	///clear work
	close_socket(sockfd);

	return 0;
}

/**
 * @return  0 on success, -1 on error
 */
int send_one_message(int sockfd, inet_address_t address, const message_t* msg)
{
	if (msg == NULL)
	{
		return -1;
	}

	int status = 0; // return value

	int recv_len = 0;
	static char recv_buf[MAX_PACK_SIZE];
	memset(recv_buf, 0, sizeof(recv_buf));

	proto_header_t* send_header = (proto_header_t*)(msg->msg_buf);

	do
	{
		status = tcp_send(sockfd, msg->msg_buf, msg->msg_buf_len, 5);
		if (status != 0)
		{
			ERROR_LOG("tcp_send to %s:%d %d bytes failed, status:%d", address.ip, address.port, msg->msg_buf_len, status);
			break;
		}

		status = tcp_recv(sockfd, FOUR_BYTES, recv_buf, sizeof(recv_buf), &recv_len, 5);
		if (status != 0)
		{
			ERROR_LOG("tcp_recv from %s:%d failed, status:%d", address.ip, address.port, status);
			break;
		}

		///检查返回报文的包头状态码
		if (recv_len >= (int)sizeof(proto_header_t))
		{
			proto_header_t* header = (proto_header_t*) recv_buf;

			INFO_LOG("sync : [%s:%d][length:%d][cmd_id:%d][user_id:%d][id:%d][status_code:%d] ---- [OK]", address.ip, address.port, msg->msg_buf_len, send_header->cmd_id, send_header->user_id, msg->id, header->status_code);

			//todo:将DB_ERR,NET_ERR作为可配置的
			if (header->status_code == 0)
			{
				//success
				break;
			}
			else if (header->status_code == DB_ERR || header->status_code == NET_ERR)
			{
				//需要重发
				status = -1;
				sleep(1);
				break;
			}
			else
			{
				WARN_LOG("message[id:%d] header status code: %d,print message in trace log", msg->id, header->status_code);
				TRACE_LOG("send_one_message: status_code=%d, message[id:%d]:", header->status_code, msg->id);
				print_in_hex(msg->msg_buf, msg->msg_buf_len);
				break;
			}
		}
		else
		{
			status = -1;
		}
	} while (false);

	//-------------------
	//return
	if (status != 0)
	{
		status = -1;
	}

	return status;
}

int fork_receive_process()
{
	pid_t pid = fork();
	if (pid == -1)
	{
		// fork error
		ERROR_LOG("fork receive process failed.");
		return -1;
	}
	else if(pid>0)
	{
		// parent
		return 0;
	}
	else
	{
		// child
		if (g_db_iface.reconnect_db())
		{
			ERROR_LOG("receive process reconnect DB failed, process exit");
			exit(0);
		}

		register_receive_sighandler();
		receive_msgs();

		INFO_LOG("=====receive process exit=====");
		exit(0);
	}

	return 0;
}

int fork_send_process()
{
	for (int i=0; i<(int)g_target_count; i++)
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			// fork error
			ERROR_LOG("fork send process [%d] failed.", i);
			return -1;
		}
		else if(pid>0)
		{
			// parent
			continue;
		}
		else
		{
			// child
			if (g_db_iface.reconnect_db())
			{
				ERROR_LOG("send process [%s:%u] reconnect DB failed, process exit", g_target_address[i].ip, g_target_address[i].port);
				exit(0);
			}

			register_send_sighandler();
			send_msgs(i);

			INFO_LOG("=====send process [%s:%d] exit=====", g_target_address[i].ip, g_target_address[i].port);
			exit(0);
		}
	}

	return 0;
}

int monitor_mq()
{
	long mq_maxmsg = 0;
	long mq_curmsgs = 0;

	while(!g_stop)
	{
		if (g_mq_mgr.get_attr(NULL, &mq_maxmsg, NULL, &mq_curmsgs) == 0)
		{
			DEBUG_LOG("mq msg number, current / max: [%ld] / [%ld]", mq_curmsgs, mq_maxmsg);
		}
		else
		{
			ERROR_LOG("get mq attribute failed");
		}

		sleep(1);
	}

	return 0;
}
