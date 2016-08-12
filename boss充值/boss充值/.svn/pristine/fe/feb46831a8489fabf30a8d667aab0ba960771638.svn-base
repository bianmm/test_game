/**
 * ==================================================================================
 * @file  	socket_util.h
 * @brief  	provide some socket operation tool funcitons
 * @author  landry
 *
 * compiler:	g++ 4.1.2
 * platform:	Linux
 *
 * copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 * ==================================================================================
 */
#ifndef _SOCKET_UTIL_H
#define _SOCKET_UTIL_H

#define MAX_PACK_SIZE (1024*8)

enum PACK_LEN
{
	TWO_BYTES = 2, FOUR_BYTES = 4
};

struct inet_address_t
{
	char ip[16];
	unsigned short port;
};

int open_socket(const char* ip, unsigned short port);

/**
 * @fn 		close_socket
 * @brief	close the socket file descriptor
 *
 * @param	sock_fd		socket file descriptor
 * @return 	return 0 on success, -1 on error
 */
int close_socket(int sock_fd);

int tcp_send(int sock_fd, const char* buf, const unsigned int buf_len, unsigned int timeout);

int tcp_recv(int sock_fd, PACK_LEN pack_len_field_len, char* buf, const unsigned int buf_len, int* recv_len, unsigned int timeout);

#endif
