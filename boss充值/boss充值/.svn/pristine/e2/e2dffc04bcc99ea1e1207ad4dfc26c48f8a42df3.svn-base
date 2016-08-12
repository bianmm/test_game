#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "cfg_file_parse.h"

static char* g_cfg_file_buffer = NULL;
static int g_cfg_file_len = 0;
static int g_cfg_file_cursor = 0;
static char g_cfg_token_buf[4096] = {0};

int cfp_init(const char* path)
{
    if (g_cfg_file_buffer != NULL) {
        return -3;
    }

    int fd, len;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    len = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);

    g_cfg_file_buffer = (char*)mmap (0, len + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (g_cfg_file_buffer == MAP_FAILED) {
        return -2;
    }

    read(fd, g_cfg_file_buffer, len);
    g_cfg_file_buffer[len] = 0;
    g_cfg_file_len = len + 1;

    close(fd);
    return 0;
}

int cfp_uninit()
{
    if ((g_cfg_file_buffer == NULL) || (g_cfg_file_len == 0)) {
        return 0;
    }

    if (munmap(g_cfg_file_buffer, g_cfg_file_len) != 0) {
        return -1;
    }

    g_cfg_file_buffer = NULL;
    g_cfg_file_len = 0;
    return 0;
}

int cfp_goto_first_line()
{
    g_cfg_file_cursor = 0;
    return 0;
}

// 注: 此函数会跳过注释行
// >0 到达文件末尾 0 成功
int cfp_goto_next_line()
{
    if ((g_cfg_file_buffer == NULL) || (g_cfg_file_len == 0) || (g_cfg_file_cursor >= g_cfg_file_len) ||
        (g_cfg_file_buffer[g_cfg_file_cursor] == '\0')) {
        return 2;
    }

    while (g_cfg_file_buffer[g_cfg_file_cursor] != '\n') {
        g_cfg_file_cursor ++;

        if ((g_cfg_file_cursor >= g_cfg_file_len) || (g_cfg_file_buffer[g_cfg_file_cursor] == '\0')) {
            return 1;
        }
    }

    g_cfg_file_cursor ++;
    if ((g_cfg_file_cursor >= g_cfg_file_len) || (g_cfg_file_buffer[g_cfg_file_cursor] == '\0')) {
        return 1;
    }

    return 0;
}

const char* cfp_get_next_token(char* buf, int buf_len)
{
    if ((g_cfg_file_buffer == NULL) || (g_cfg_file_len == 0) || (g_cfg_file_cursor >= g_cfg_file_len) ||
        (g_cfg_file_buffer[g_cfg_file_cursor] == '\0')) {
        return NULL;
    }

    // 跳过空格
    while ((g_cfg_file_buffer[g_cfg_file_cursor] == ' ') || (g_cfg_file_buffer[g_cfg_file_cursor] == '\t')) {
        if ((g_cfg_file_cursor >= g_cfg_file_len) || (g_cfg_file_buffer[g_cfg_file_cursor] == '\0') ||
            (g_cfg_file_buffer[g_cfg_file_cursor] == '\r') || (g_cfg_file_buffer[g_cfg_file_cursor] == '\n')) {
            return NULL;
        }

        g_cfg_file_cursor ++;
    }

    // #号为注释
    if (g_cfg_file_buffer[g_cfg_file_cursor] == '#') {
        return NULL;
    }

    int tk_start = g_cfg_file_cursor;

    // 直到遇到空格、换行或文件尾
    while ((g_cfg_file_buffer[g_cfg_file_cursor] != ' ') && (g_cfg_file_buffer[g_cfg_file_cursor] != '\t') &&
           (g_cfg_file_cursor < g_cfg_file_len) && (g_cfg_file_buffer[g_cfg_file_cursor] != '\0') &&
           (g_cfg_file_buffer[g_cfg_file_cursor] != '\r') && (g_cfg_file_buffer[g_cfg_file_cursor] != '\n')) {
        g_cfg_file_cursor ++;
    }

    int tk_len = g_cfg_file_cursor - tk_start;
    if (tk_len <= 0) {
        return NULL;
    }

    if (buf != NULL) {
        tk_len = tk_len  > (buf_len - 1) ? (buf_len - 1) : tk_len;
        strncpy(buf, g_cfg_file_buffer + tk_start, tk_len);
        buf[tk_len] = '\0';
        return (const char*)buf;
    }

    // 如果用户不指定缓冲区，则token长度不允许大于4096
    if (tk_len + 1 > 4096) {
        return NULL;
    }

    strncpy(g_cfg_token_buf, g_cfg_file_buffer + tk_start, tk_len);
    g_cfg_token_buf[tk_len] = '\0';
    return (const char*)g_cfg_token_buf;
}
