#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json/json.h>

#include <string>

#include "appstore.h"
#include "benchapi.h"


SSL_Conn_t conn_notify[2] = {
{NULL,NULL,-1,2195,"gateway.push.apple.com"},
{NULL,NULL,-1,2195,"gateway.sandbox.push.apple.com"}
};

SSL_Conn_t conn_receipt[2] = {
{NULL,NULL,-1,443,"buy.itunes.apple.com"},
{NULL,NULL,-1,443,"sandbox.itunes.apple.com"}
};

char g_receipt_send_buf[10240] = {0};
char g_receipt_recv_buf[10240] = {0};

void* Receipt_Thread(void*);

void Free_SSL_CTX()
{
	if(conn_receipt[IAP_SANDBOX].pctx != NULL) {
		SSL_CTX_free(conn_receipt[IAP_SANDBOX].pctx);
		conn_receipt[IAP_SANDBOX].pctx = NULL;
		conn_receipt[IAP_SANDBOX].pctx = NULL;
	}
    
    if(conn_receipt[IAP_PRODUCT].pctx != NULL) {
        SSL_CTX_free(conn_receipt[IAP_PRODUCT].pctx);
        conn_receipt[IAP_PRODUCT].pctx = NULL;
        conn_receipt[IAP_PRODUCT].pctx = NULL;
    }
}

void Free_SSL_Conn(SSL_Conn_t* conn)
{
	if(conn->pssl) {
		SSL_shutdown(conn->pssl);
		SSL_free(conn->pssl);
		close(conn->sock);
		conn->pssl = NULL;
		conn->sock = -1;
	}
}

int Try_Connect(SSL_Conn_t* conn)
{
	int err;
	int bytes;
	fd_set	rfds;	
	char buf[1024];
	struct hostent *hp;
	struct sockaddr_in sa;
	struct timeval timeout;

	if(conn->pssl) {
reselect:
		FD_ZERO(&rfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;
		FD_SET(conn->sock,&rfds);
		err = select(conn->sock+1,&rfds,NULL,NULL,&timeout);
		if(err == -1) {
			DEBUG_LOG("select error");
			Free_SSL_Conn(conn);
		} else if(err == 0) {
			return 0;
		} else {
			bytes = SSL_read(conn->pssl,buf,1020);
			if(bytes > 0) {
				buf[bytes] = 0;
				DEBUG_LOG("SSL_read bytes=%d %s",bytes,buf);
			}
			err = SSL_get_error(conn->pssl,bytes);
			switch(err) {
				case SSL_ERROR_NONE:
				case SSL_ERROR_WANT_WRITE:
					return 0;
				case SSL_ERROR_WANT_READ:
					goto reselect;
				case SSL_ERROR_ZERO_RETURN:
					DEBUG_LOG("Connection Closed bytes=%d",bytes);
					Free_SSL_Conn(conn);
					break;
				default:
					DEBUG_LOG("%s,bytes=%d",ERR_error_string(err,0),bytes);
					Free_SSL_Conn(conn);
					break;
			}
		}
	}

	DEBUG_LOG("Try_Connect %s:%d ...",conn->host,conn->port);

	memset(&sa,0,sizeof(sa));
	hp = gethostbyname(conn->host);
    if(!hp) {
		DEBUG_LOG("Couldn't Resolve %s",conn->host);
		return -1;
	}

	sa.sin_family	= AF_INET;
	sa.sin_port		= htons(conn->port);
	memcpy(&sa.sin_addr,hp->h_addr,sizeof(sa.sin_addr));

	conn->pssl = SSL_new(conn->pctx);
	if (conn->pssl == NULL) {
		DEBUG_LOG("Try_Connect %s:%d SSL_new",conn->host,conn->port);
		return -1;
	}

	conn->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (conn->sock < 0) {
		DEBUG_LOG("Try_Connect %s:%d socket",conn->host,conn->port);
		Free_SSL_Conn(conn);
		return -1;
	}

	err = connect(conn->sock,(struct sockaddr*)&sa,sizeof(sa));
	if (err == -1) {
		DEBUG_LOG("Try_Connect %s:%d Failed",conn->host,conn->port);
		Free_SSL_Conn(conn);
		return -1;
	}

	SSL_set_fd(conn->pssl,conn->sock);
	err = SSL_connect(conn->pssl);
	if (err <= 0) {
	    DEBUG_LOG("%s",ERR_error_string(SSL_get_error(conn->pssl,err),NULL));
	    Free_SSL_Conn(conn);
	    return -1;
	}

	return 0;
}

int Init_SSL_CTX(iap_env_type_t iap_env)
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	conn_receipt[iap_env].pctx = SSL_CTX_new(SSLv23_client_method());

	if (conn_receipt[iap_env].pctx == NULL) {
		DEBUG_LOG("SSL_CTX_new");
		return -1;
	}

	if(Try_Connect(&conn_receipt[iap_env])) return -1;

	return 0;
}

int Try_Send(SSL_Conn_t* conn, char *data, int len)
{
	int n = 0;
	int bytes = 0;

	if(Try_Connect(conn)) return -1;

	while(len > 0) {
		bytes = SSL_write(conn->pssl, &data[n], len - n);
		if(bytes <= 0) {
			int ret = SSL_get_error(conn->pssl,bytes);
			switch(ret) {
				case SSL_ERROR_NONE:
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_ZERO_RETURN:
					DEBUG_LOG("Connection Closed  n=%d",n);
					Free_SSL_Conn(conn);
					return -1;
				default:
					DEBUG_LOG("%s",ERR_error_string(ret,0));
					Free_SSL_Conn(conn);
					return -1;
			}
		} else {
			n += bytes;
			len -= bytes;
		}
	}

	return 0;
}

int Receipt_Status(char *post, int bytes, char* rsp_buf, int rsp_buf_len, char* rsp_pdt_id, int rsp_pdt_len, iap_env_type_t iap_env)
{
	int n = 0;
	int status = 0;
	long hdr_len = 0;
	int ctx_len = 0;
	char *ptr = NULL;
	SSL_Conn_t* conn = &conn_receipt[iap_env];
    int retry_cnt = 0;

retry_send:
    
    rsp_buf[0] = '\0';
    rsp_pdt_id[0] = '\0';
    
    if (Try_Send(conn, post, bytes) != 0) {
        ERROR_LOG("Try_Send fail");
        Free_SSL_Conn(conn);
        
        // 这里重试3次
        retry_cnt ++;
        if (retry_cnt <= 3) {
            DEBUG_LOG("retry send: %d", retry_cnt);
            goto retry_send;
        }
        
        return -1;
    }
    
    memset(g_receipt_recv_buf, 0, sizeof(g_receipt_recv_buf));
	while(1) {
		bytes = SSL_read(conn->pssl,&g_receipt_recv_buf[n],4096 - n);
		if(bytes <= 0) {
			int ret = SSL_get_error(conn->pssl,bytes);
			switch(ret) {
				case SSL_ERROR_NONE:
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					break;
				case SSL_ERROR_ZERO_RETURN:
					ERROR_LOG("Connection Closed n=%d",n);
					Free_SSL_Conn(conn);
                    
                    // 这里重试3次
                    retry_cnt ++;
                    if (retry_cnt <= 3) {
                        DEBUG_LOG("retry send: %d", retry_cnt);
                        goto retry_send;
                    }
                    
					return -1;
				default:
					ERROR_LOG("SSL err: %d,%d,%s",ret,errno,ERR_error_string(ret,0));
					Free_SSL_Conn(conn);
                    
                    // 这里重试3次
                    retry_cnt ++;
                    if (retry_cnt <= 3) {
                        DEBUG_LOG("retry send: %d", retry_cnt);
                        goto retry_send;
                    }
                    
					return -1;
			}
		} else {
			n += bytes;
			g_receipt_recv_buf[n] = 0;
			if(hdr_len == 0) {
				ptr = strstr(g_receipt_recv_buf,"\r\n\r\n");
				if(ptr) {
					hdr_len = (ptr - g_receipt_recv_buf) + 4;
					ptr = strcasestr(g_receipt_recv_buf,"content-length");
					ctx_len = atoi(strstr(ptr,":") + 1);
					n -= hdr_len;
					memmove(g_receipt_recv_buf,g_receipt_recv_buf + hdr_len,n + 1);
				}
			}
			if(ctx_len > 0 && n >= ctx_len) {
                DEBUG_LOG("Receipt post: %s\nReceipt resp: %s", post, g_receipt_recv_buf);
				ptr = strcasestr(g_receipt_recv_buf,"\"status\"");
				if(ptr) {
					status = atoi(strstr(ptr,":") + 1);
					if(status == 21005) status = 0;
				} else {
					status = 0;
				}
                
                if (status != 0) {
                    break;
                }
                
                // 查找resp
                ptr = strcasestr(g_receipt_recv_buf,"\"receipt\"");
                if (ptr) {
                    ptr = strstr(ptr,":") + 1;
                    long len = strstr(ptr, "}") - ptr + 1;
                    if (len > rsp_buf_len) {
                        ERROR_LOG("len %d > rsp_buf_len %d", len, rsp_buf_len);
                        return -1;
                    }
                    else {
                        memcpy(rsp_buf, ptr, len);
                        rsp_buf[len] = '\0';
                        
                        // 查找pdt_id
                        ptr = strcasestr(rsp_buf,"\"product_id\"");
                        ptr = strstr(ptr,":");
                        ptr = strstr(ptr,"\"") + 1;
                        len = strstr(ptr, "\"") - ptr;
                        if (len > rsp_pdt_len) {
                            ERROR_LOG("len %d > rsp_pdt_len %d", len, rsp_pdt_len);
                            return -1;
                        }
                        else {
                            memcpy(rsp_pdt_id, ptr, len);
                            rsp_pdt_id[len] = '\0';
                            break;
                        }
                    }
                }
			}
		}
	}

	return status;
}

int Get_Receipt_Status(char *receipt, char* rsp_buf, int rsp_buf_len, char* rsp_pdt_id, int rsp_pdt_len, iap_env_type_t iap_env)
{
    memset(g_receipt_send_buf, 0, sizeof(g_receipt_send_buf));
    sprintf(g_receipt_send_buf, "POST /verifyReceipt HTTP/1.1\r\nAccept: */*\r\nAccept-Language:UTF-8\r\nConnection:keep-alive\r\nContent-Length:%lu\r\n\r\n{\"receipt-data\":\"%s\"}", strlen(receipt) + 19, receipt);
	return Receipt_Status(g_receipt_send_buf, (int)strlen(g_receipt_send_buf), rsp_buf, rsp_buf_len, rsp_pdt_id, rsp_pdt_len, iap_env);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int Get_Receipt_Status_curl(char *receipt, char* rsp_buf, int rsp_buf_len, char* rsp_pdt_id, int rsp_pdt_len, bool is_sandbox)
{
    int ret = -1;
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    curl = curl_easy_init();
    if(curl) {
        if (is_sandbox) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox.itunes.apple.com/verifyReceipt");
        }
        else {
            curl_easy_setopt(curl, CURLOPT_URL, "https://buy.itunes.apple.com/verifyReceipt/");
        }
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        
        memset(g_receipt_send_buf, 0, sizeof(g_receipt_send_buf));
        sprintf(g_receipt_send_buf, "{\"receipt-data\":\"%s\"}", receipt);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, g_receipt_send_buf);
        
        DEBUG_LOG("Verify request: %s", g_receipt_send_buf);
        
        int retry_cnt = 0;
        
    retry:
        readBuffer.clear();
        
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            retry_cnt ++;
            if (retry_cnt <= 3) {
                ERROR_LOG("curl_easy_perform() failed: %s, retry %d",
                          curl_easy_strerror(res), retry_cnt);
                goto retry;
            }
            else {
                ERROR_LOG("curl_easy_perform() failed: %s, max retry",
                          curl_easy_strerror(res));
            }
        }
        else {
            DEBUG_LOG("Verify resp: %s", readBuffer.c_str());
            
            json_object* jobj = json_tokener_parse(readBuffer.c_str());
            
            json_object* status_obj;
            json_object* resp_obj;
            if (json_object_object_get_ex(jobj, "status", &status_obj)) {
                int status = json_object_get_int(status_obj);
                if (status == 0) {
                    if (json_object_object_get_ex(jobj, "receipt", &resp_obj)) {
                        strcpy(rsp_buf, json_object_get_string(resp_obj));
                        
                        json_object* pdt_obj;
                        if (json_object_object_get_ex(resp_obj, "product_id", &pdt_obj)) {
                            strcpy(rsp_pdt_id, json_object_get_string(pdt_obj));
                            
                            DEBUG_LOG("got status %d, pdt: %s", status, rsp_pdt_id);
                        }
                        else {
                            ERROR_LOG("can not found product_id obj");
                        }
                    }
                    else {
                        ERROR_LOG("can not found resp obj");
                    }
                }
                
                ret = status;
            }
            else {
                retry_cnt ++;
                if (retry_cnt <= 3) {
                    ERROR_LOG("can not found status obj, retry %d", retry_cnt);
                    goto retry;
                }
                else {
                    ERROR_LOG("can not found status obj, max retry");
                }
                
                //ERROR_LOG("can not found status obj");
            }
        }
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    
    return ret;
}