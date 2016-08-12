
#ifndef _CMD_HANDLER_H
#define _CMD_HANDLER_H

#include <stdint.h>

int anypay_query(uint32_t user_id,
                    const char *req_body_buf,
                    int req_body_len,
                    char *rsp_body_buf,
                    int *rsp_body_len,
                    uint32_t *status_code);

int anypay_preorder(uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *rsp_body_buf,
        int *rsp_body_len,
        uint32_t *status_code);

int anypay_notify(int cmd_id,
        uint32_t user_id,
        const char *req_body_buf,
        int req_body_len,
        char *rsp_body_buf,
        int *rsp_body_len,
        uint32_t *status_code);

int anypay_notify_client(int cmd_id,
                  uint32_t user_id,
                  const char *req_body_buf,
                  int req_body_len,
                  char *rsp_body_buf,
                  int *rsp_body_len,
                  uint32_t *status_code);

#endif
