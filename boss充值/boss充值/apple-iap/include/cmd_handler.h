
#ifndef _CMD_HANDLER_H
#define _CMD_HANDLER_H

#include <stdint.h>

int apple_preorder(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code);

int receipt_verify(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *ack_body_buf,
        int *ack_body_len,
        uint32_t *status_code);

#endif
