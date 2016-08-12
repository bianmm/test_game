/**
 * =====================================================================================
 *       @file  outer_connection_mgr.cpp
 *      @brief  提供短连接交互功能的类
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  01/31/2010 12:41:06 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2009, TaoMee.Inc, ShangHai.
 *
 *     @author  jasonwang (王国栋) jasonwang@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */

#include "outer_connection_mgr.h"

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>

#include "benchapi.h"

outer_connection_mgr_t::outer_connection_mgr_t()
    : m_inited(false), m_port(0)
{
    memset(m_addr, 0, sizeof(m_addr));
}

outer_connection_mgr_t::~outer_connection_mgr_t()
{
    uninit();
}

int outer_connection_mgr_t::init(const char* addr, unsigned short port)
{
    if(m_inited) {
        return -1;
    }

    if((addr == NULL) || (port <= 0)) {
        return -1;
    }

    if(strlen(addr) >= sizeof(m_addr)) {
        return -1;
    }

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        return -1;
    }

    sockaddr_in remote_addr;
    memset(&remote_addr ,0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(addr);

    if(connect(conn_fd, (sockaddr*)&remote_addr, sizeof(remote_addr))) {
        close(conn_fd);
        return -1;
    }

    close(conn_fd);

    strcpy(m_addr, addr);
    m_port = port;

    m_inited = true;

    return 0;
}

int outer_connection_mgr_t::ping()
{
    if(! m_inited) {
        return -1;
    }

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        return -3;
    }

    sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(m_port);
    remote_addr.sin_addr.s_addr = inet_addr(m_addr);

    if(connect(conn_fd, (sockaddr*)&remote_addr, sizeof(remote_addr))) {
        close(conn_fd);
        return -5;
    }

    close(conn_fd);
    return 0;
}

int outer_connection_mgr_t::do_io(const char* snd_buf, int snd_len, char* rcv_buf, int rcv_len, int timed_out, size_t pkg_len_size)
{
    if(! m_inited) {
        return -1;
    }

    if((snd_buf == NULL) || (snd_len <= 0)) {
        return -2;
    }

    if ((pkg_len_size != 2) && (pkg_len_size != 4)) {
        return -3;
    }

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        return -4;
    }

    sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(m_port);
    remote_addr.sin_addr.s_addr = inet_addr(m_addr);

    if(connect(conn_fd, (sockaddr*)&remote_addr, sizeof(remote_addr))) {
        close(conn_fd);
        return -5;
    }

    int bytes = send(conn_fd, snd_buf, snd_len, 0);
    if(bytes < 0) {
        close(conn_fd);
        return -6;
    }

    if((rcv_len > 0) && (rcv_buf != NULL)) {
        int bytes_rcved = 0;
        int time_sleeped = 0; /// millseconds

        int flags = fcntl(conn_fd,F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(conn_fd, F_SETFL, flags);

        int pkg_len = 0;

        while (true) {
            bytes = recv(conn_fd, rcv_buf + bytes_rcved, rcv_len - bytes_rcved, 0);
            if(bytes > 0) {
                bytes_rcved += bytes;

                /// 接收包体长度
                if ((pkg_len == 0) && ((size_t)bytes_rcved >= pkg_len_size)) {
                    memcpy(&pkg_len, rcv_buf, pkg_len_size);

                    /// 保证安全
                    if (pkg_len > rcv_len) {
                        return -7;
                    }
                }

                if (bytes_rcved >= pkg_len) {
                    close(conn_fd);
                    return 0;
                }
            }
            else if(bytes == 0) {
                /// remote has closed gracefully
                close(conn_fd);
                return -8;
            }
            else {
                /// has error
                if(errno == EINTR) {
                    continue;
                }
                else if(errno == EAGAIN) {
                    timespec time_interval;
                    time_interval.tv_sec = 0;
                    time_interval.tv_nsec = 1000000; /// 1 millseconds

                    nanosleep(&time_interval,NULL);
                    time_sleeped += 1;

                    if(time_sleeped >= timed_out * 1000) {
                        /// timed out
                        close(conn_fd);
                        return -9;
                    }
                }
                else
                {
                    /// critical error
                    close(conn_fd);
                    return -10;
                }
            }
        }
    }
    else
    {
        /// no need to recv data
        close(conn_fd);
        return 0;
    }
}

int outer_connection_mgr_t::do_io_text(const char* snd_buf, int snd_len, char* rcv_buf, int rcv_len, int timed_out)
{
    if(! m_inited) {
        return -1;
    }

    if((snd_buf == NULL) || (snd_len <= 0)) {
        return -2;
    }

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        return -4;
    }

    sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(m_port);
    remote_addr.sin_addr.s_addr = inet_addr(m_addr);

    if(connect(conn_fd, (sockaddr*)&remote_addr, sizeof(remote_addr))) {
        close(conn_fd);
        return -5;
    }

    int bytes = send(conn_fd, snd_buf, snd_len, 0);
    if(bytes < 0) {
        close(conn_fd);
        return -6;
    }

    if((rcv_len > 0) && (rcv_buf != NULL)) {
        int time_sleeped = 0; /// millseconds

        int flags = fcntl(conn_fd,F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(conn_fd, F_SETFL, flags);

        int body_len = 0;
        char c_buf = 0;
        int rcv_buf_idx = 0;
        int status = 0; // 1 正在读取\n\n 2 正在读取长度 3 长度读取完毕
        char len_buf[20] = {0};
        int len_buf_idx = 0;
        while (true) {
            bytes = recv(conn_fd, &c_buf, 1, 0);
            if(bytes > 0) {
                if (status == 0) {
                    if (c_buf == '\n') {
                        status = 1;
                    }
                }
                else if (status == 1) {
                    if (c_buf == '\n') {
                        status = 2;
                    }
                    else {
                        // error
                        status = 0;
                    }
                }
                else if (status == 2) {
                    if (c_buf == ':') {
                        len_buf[len_buf_idx] = '\0';
                        body_len = atoi(len_buf);
                        status = 3;
                    }
                    else {
                        len_buf[len_buf_idx++] = c_buf;
                    }
                }
                else if (status == 3) {
                    rcv_buf[rcv_buf_idx++] = c_buf;

                    if (rcv_buf_idx > rcv_len) {
                        // error
                        return -7;
                    }
                    if (rcv_buf_idx > body_len) {
                        // success
                        rcv_buf[rcv_buf_idx] = '\0';
                        return 0;
                    }
                }
                else {
                    // error
                    return -8;
                }
            }
            else if(bytes == 0) {
                /// remote has closed gracefully
                close(conn_fd);
                return -8;
            }
            else {
                /// has error
                if(errno == EINTR) {
                    continue;
                }
                else if(errno == EAGAIN) {
                    timespec time_interval;
                    time_interval.tv_sec = 0;
                    time_interval.tv_nsec = 1000000; /// 1 millseconds

                    nanosleep(&time_interval,NULL);
                    time_sleeped += 1;

                    if(time_sleeped >= timed_out * 1000) {
                        /// timed out
                        close(conn_fd);
                        return -9;
                    }
                }
                else
                {
                    /// critical error
                    close(conn_fd);
                    return -10;
                }
            }
        }
    }
    else
    {
        /// no need to recv data
        close(conn_fd);
        return 0;
    }
}


int outer_connection_mgr_t::uninit()
{
    if(! m_inited) {
        return -1;
    }

    memset(m_addr, 0, sizeof(m_addr));

    m_port = 0;
    m_inited = false;

    return 0;
}

