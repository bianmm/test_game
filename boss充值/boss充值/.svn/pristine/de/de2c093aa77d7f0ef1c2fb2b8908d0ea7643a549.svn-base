
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

const char *get_table_name(const char *email);

int atoi_safe(const char *nptr);

const char *trim_str(char *str);

void md5_16to32(const unsigned char *p_md5_buffer_16,
                unsigned char *p_md5_buffer_32,
                int is_upper);

/// 校验头
typedef struct {
    uint16_t chnl_id;
    uint8_t vfy_code[32];
} __attribute__((packed)) chnlhash32_vfy_header_t;

int fill_chnlhash32(chnlhash32_vfy_header_t*, uint16_t, const char*, const char*, int);
int verify_chnlhash32(const chnlhash32_vfy_header_t*, const char*, const char*, int);

#endif
