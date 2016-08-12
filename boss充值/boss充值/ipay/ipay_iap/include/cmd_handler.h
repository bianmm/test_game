
#ifndef _CMD_HANDLER_H
#define _CMD_HANDLER_H

#include <stdint.h>

int ipay_preorder(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *rsp_body_buf,
        int *rsp_body_len,
        uint32_t *status_code);

int ipay_notify(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *rsp_body_buf,
        int *rsp_body_len,
        uint32_t *status_code);

#endif
