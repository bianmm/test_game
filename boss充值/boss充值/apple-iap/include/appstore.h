#ifndef _LIB_APN_HPP_
#define _LIB_APN_HPP_

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef enum
{
    IAP_PRODUCT = 0,
    IAP_SANDBOX = 1,
} iap_env_type_t;

typedef struct {
    SSL_CTX*	pctx;
    SSL*		pssl;
    int			sock;
    uint16_t	port;
    const char*	host;
} SSL_Conn_t;

int Init_SSL_CTX(iap_env_type_t iap_env);
int Get_Receipt_Status(char *receipt, char* rsp_buf, int rsp_buf_len, char* rsp_pdt_id, int rsp_pdt_len, iap_env_type_t iap_env);
void Free_SSL_Conn(SSL_Conn_t* conn);

int Get_Receipt_Status_curl(char *receipt, char* rsp_buf, int rsp_buf_len, char* rsp_pdt_id, int rsp_pdt_len, bool is_sandbox);

#endif
