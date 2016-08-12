/**
 * ==================================================================================
 * @file  	socket_util.cpp
 * @brief  	provide some socket operation tool funcitons
 * @author  landry
 *
 * compiler:	g++ 4.1.2
 * platform:	Linux
 *
 * copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 * ==================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "socket_util.h"
#include "log.h"

/**
 * @fn 		open_socket
 * @brief	open the socket connection to the host with the specified ip address and port.
 *
 * @param	ip		ip address
 * @param	port	port number
 * @return 	return the socket descriptor on success, -1 on error
 */
int open_socket(const char* ip, unsigned short port)
{
	int sock_fd;
	struct sockaddr_in svr_addr;

	bzero(&svr_addr, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &svr_addr.sin_addr);

	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (connect(sock_fd, (const struct sockaddr *) &svr_addr, sizeof(svr_addr)) != 0)
	{
		close(sock_fd);
		return -1;
	}

	return sock_fd;
}

/**
 * @fn 		close_socket
 * @brief	close the socket file descriptor
 *
 * @param	sock_fd		socket file descriptor
 * @return 	return 0 on success, -1 on error
 */
int close_socket(int sock_fd)
{
	return close(sock_fd);
}

/**
 *
 * @param  timeout the timeout second, if timeout is zero there will be no timeout limit
 * @return 0 on success, -1 on error, 1 on timeout
 */
int tcp_send(int sock_fd, const char* buf, const unsigned int buf_len, unsigned int timeout)
{
	if (buf == NULL)
	{
		return -1;
	}

	if (buf_len == 0)
	{
		return 0;
	}

	unsigned int start_time = time(NULL);
	int cur_len = 0;

	for (int sent_bytes = 0; sent_bytes < (int)buf_len; sent_bytes += cur_len)
	{
		if (timeout > 0 && time(NULL) > start_time + timeout)
		{
			//timeout
			return 1;
		}

		cur_len = send(sock_fd, buf + sent_bytes, buf_len - sent_bytes, MSG_DONTWAIT); // send in non-blocking mode

		if (cur_len == -1)
		{
			if (errno == EAGAIN)
			{
				// send() would block
				cur_len = 0;
				continue;
			}
			else if (errno == EINTR)
			{
				// interrupted by a signal
				cur_len = 0;
				continue;
			}
			else
			{
				//error
				return -1;
			}
		}
	}

	return 0;
}

/**
 *
 * @param  pack_len_field_len : the bytes of package length description field in message header.
 * @param  timeout : the timeout second, if timeout is zero there will be no timeout limit
 * @return 0 on success, -1 on error, 1 on timeout
 */
int tcp_recv(int sock_fd, PACK_LEN pack_len_field_len, char* buf, const unsigned int buf_len, int* recv_len, unsigned int timeout)
{
	if (buf == NULL || recv_len == NULL)
	{
		return -1;
	}

	if (buf_len == 0)
	{
		return -1;
	}

	unsigned int start_time = time(NULL);

	unsigned int pack_len = pack_len_field_len;  // The package length value in the message header.
	bool get_pack_len = false;  // Indicate if already get the package length value in the message header.

	int cur_recv_bytes = 0;  // Already received bytes' count.
	int cur_len = 0;

	while (cur_recv_bytes < (int)pack_len)
	{
		if (timeout > 0 && time(NULL) > start_time + timeout)
		{
			//timeout
			return 1;
		}

		cur_len = recv(sock_fd, (buf + cur_recv_bytes), (pack_len - cur_recv_bytes), MSG_DONTWAIT);  // recv in non-blocking mode

		if (cur_len > 0)
		{
			cur_recv_bytes += cur_len;

			if (!get_pack_len && cur_recv_bytes >= pack_len_field_len)
			{
				// Get package length declared in the package header.
				if (pack_len_field_len == TWO_BYTES)
				{
					get_pack_len = true;
					pack_len = *((uint16_t*) buf);
					if (pack_len > MAX_PACK_SIZE)
						pack_len = MAX_PACK_SIZE;
				}
				else if (pack_len_field_len == FOUR_BYTES)
				{
					get_pack_len = true;
					pack_len = *((uint32_t*) buf);
					if (pack_len > MAX_PACK_SIZE)
						pack_len = MAX_PACK_SIZE;
				}
				else
				{
					return -1;
				}
			}
		}
		else if (cur_len == 0)
		{
			// peer closed gracefully.
			return -1;
		}
		else
		{
            sleep(1);
			if (errno == EINTR)
			{
				// interrupted by a signal.
				cur_len = 0;
				continue;
			}
			else if (errno == EAGAIN)
			{
				// recv() would block
				cur_len = 0;
				continue;
			}
			else
			{
				return -1;
			}
		}
	} // For end

	*recv_len = cur_recv_bytes;

	return 0;
}
