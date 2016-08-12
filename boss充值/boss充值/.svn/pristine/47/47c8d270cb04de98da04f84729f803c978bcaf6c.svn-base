#ifndef WORK_H
#define WORK_H

#include <errno.h>
#include <unistd.h>

#include "mq_mgr.h"
#include "log.h"
#include "socket_util.h"
#include "util.h"
#include "db_iface.h"

#define MAX_TARGET_COUNT     5    // the max count of target systems
#define BATCH_RECEIVE_COUNT  100  //
#define BATCH_QUERY_COUNT    10   //

#define DB_ERR					1002 //数据库出错
#define NET_ERR					1003 //NET出错

//报文包头结构
struct dbproxy_proto_header_t{
    uint32_t pack_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed));

typedef dbproxy_proto_header_t proto_header_t; //目前proto_header_t就是dbprdbproxy_proto_header_t

int wait_children();

void set_stop_flag(int signo);

void wait_child(int signo);

int register_main_sighandler();

int register_receive_sighandler();

int register_send_sighandler();

int receive_msgs();

int send_msgs(int index);

int send_one_message(int sockfd, inet_address_t address, const message_t* msg);

int fork_send_process();

int fork_receive_process();

int monitor_mq();

#endif
