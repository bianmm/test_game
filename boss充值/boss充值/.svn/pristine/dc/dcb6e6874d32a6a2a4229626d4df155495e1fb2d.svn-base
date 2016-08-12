#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <openssl/md5.h>

#include "benchapi.h"
#include "functions.h"

/// hash 缓冲
#define MAX_HASH_BUF_LEN 4096
char g_hash_buf[MAX_HASH_BUF_LEN];
uint8_t g_md5_sum[32+1];

int atoi_safe(const char *nptr)
{
	if (nptr == NULL) {
		return 0;
	}

	return atoi(nptr);
}

const char *trim_str(char *str)
{
    int len = strlen(str);
    int start = 0;
    int end = len - 1;

    for (start = 0; start < len; start++) {
        if (!isspace(str[start])) {
            break;
        }
    }

    for (end = len - 1; end >= 0; end--) {
        if (!isspace(str[end])) {
            break;
        }
    }

    int i, j;
    for (i = 0, j = start; i < len && j <= end; i++, j++) {
        str[i] = str[j];
    }
    str[i] = '\0';

    return str;
}

void md5_16to32(const unsigned char *p_md5_buffer_16,
                unsigned char *p_md5_buffer_32,
                int is_upper)
{
    const char *lower_set = "0123456789abcdef";
    const char *upper_set = "0123456789ABCDEF";
    
    if (is_upper)
    {
        for (int i = 0; i < 16; ++ i)
        {
            p_md5_buffer_32[2 * i]
            = upper_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
            p_md5_buffer_32[2 * i + 1]
            = upper_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
        }
    }
    else
    {
        for (int i = 0; i < 16; ++ i)
        {
            p_md5_buffer_32[2 * i]
            = lower_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
            p_md5_buffer_32[2 * i + 1]
            = lower_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
        }
    }
    
    return;
}

char get_hex_asc_lower(int i)
{
    if ( i<0 || i>15)
        return ' ';
    if ( i < 10 )
        return ('0' + i);
    if ( i >= 10 )
        return ('a' + i - 10);
    
    return ' ';
}

int hex2asc_lower(uint8_t* dst, size_t len, const uint8_t* src)
{
    for(size_t i=0; i<len; i++) {
        uint8_t tmp = (i%2==0 ? src[i/2] >> 4 : src[i/2] & 0x0F);
        dst[i]=get_hex_asc_lower(tmp);
    }
    
    return 0;
}

const uint8_t* md5_sum_32(const uint8_t* pkg, size_t pkg_len)
{
    uint8_t s_raw_md5_sum[16];
    
    MD5(pkg, pkg_len, s_raw_md5_sum);
    hex2asc_lower(g_md5_sum, 32, s_raw_md5_sum);
    
    /// 保证字符串正常结束，确保安全
    g_md5_sum[32] = '\0';
    
    return g_md5_sum;
}

int fill_chnlhash32(chnlhash32_vfy_header_t* vfh, uint16_t chnl_id, const char* key, const char* data, int data_len)
{
    vfh->chnl_id = chnl_id;
    
    int hash_body_len = sprintf(g_hash_buf, "channelId=%d&securityCode=%s&data=", vfh->chnl_id, key);
    if (hash_body_len <= 0) {
        ERROR_LOG("Verify hash: sprintf error.");
        return -1;
    }
    
    memcpy(g_hash_buf + hash_body_len, data, data_len);
    hash_body_len += data_len;
    memcpy(vfh->vfy_code, md5_sum_32((uint8_t*)g_hash_buf, hash_body_len), 32);
    
    return 0;
}

int verify_chnlhash32(const chnlhash32_vfy_header_t* vfh, const char* key, const char* data, int data_len)
{
    int hash_body_len = sprintf(g_hash_buf, "channelId=%d&securityCode=%s&data=", vfh->chnl_id, key);
    if (hash_body_len <= 0) {
        ERROR_LOG("Verify hash: sprintf error.");
        return -1;
    }
    
    memcpy(g_hash_buf + hash_body_len, data, data_len);
    hash_body_len += data_len;
    
    if (memcmp(vfh->vfy_code, md5_sum_32((uint8_t*)g_hash_buf, hash_body_len), 32) == 0)
        return 0;
    
    return -1;
}
