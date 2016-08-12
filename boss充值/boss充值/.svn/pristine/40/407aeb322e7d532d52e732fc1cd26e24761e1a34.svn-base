#include <zlib.h>

extern const char* GET_SVR_PROTO_STR_ZIP_CMD;
extern const char* GET_SVR_PROTO_STR_RAW_CMD;

int get_server_proto_xml_zip(Bytef** buf, uLongf* len);
const char* get_server_proto_xml_str();

#define CHECK_IS_GET_SERVER_PTOTO_CMD(buf, len) \
    if ((strncmp(buf, GET_SVR_PROTO_STR_ZIP_CMD, strlen(GET_SVR_PROTO_STR_ZIP_CMD)) == 0)) { \
        return strlen(GET_SVR_PROTO_STR_ZIP_CMD); \
    } \
    if ((strncmp(buf, GET_SVR_PROTO_STR_RAW_CMD, strlen(GET_SVR_PROTO_STR_RAW_CMD)) == 0)) { \
        return strlen(GET_SVR_PROTO_STR_RAW_CMD); \
    }

#define RETURN_IF_IS_GET_SERVER_PROTO_CMD(buf, len, snd_buf, snd_buf_len) \
    if ((strncmp(buf, GET_SVR_PROTO_STR_ZIP_CMD, strlen(GET_SVR_PROTO_STR_ZIP_CMD)) == 0)) { \
        uLongf len = 0; \
        Bytef* zlib_buf = NULL; \
        uint32_t val = 0; \
        if ((get_server_proto_xml_zip(&zlib_buf, &len) != 0) || (zlib_buf == NULL) || (len == 0)) { \
            val = -1; \
            memcpy(snd_buf, &val, sizeof(val)); \
            snd_buf_len = sizeof(val); \
        } \
        else { \
            val = 0; \
            memcpy(snd_buf, &val, sizeof(val)); \
            val = len; \
            memcpy(snd_buf + sizeof(val), &val, sizeof(val)); \
            memcpy(snd_buf + sizeof(val) * 2, zlib_buf, len); \
            snd_buf_len = sizeof(val) * 2 + len; \
        } \
        return 0; \
    } \
    if ((strncmp(buf, GET_SVR_PROTO_STR_RAW_CMD, strlen(GET_SVR_PROTO_STR_RAW_CMD)) == 0)) { \
        uint32_t val = 0; \
        memcpy(snd_buf, &val, sizeof(val)); \
        const char* xml_str = get_server_proto_xml_str(); \
        val = strlen(xml_str); \
        memcpy(snd_buf + sizeof(val), &val, sizeof(val)); \
        memcpy(snd_buf + sizeof(val) * 2, xml_str, val); \
        snd_buf_len = sizeof(val) * 2 + val; \
        return 0; \
    }

