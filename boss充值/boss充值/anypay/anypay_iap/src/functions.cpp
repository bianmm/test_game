
#include <ctype.h>
#include <openssl/md5.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>

#include "functions.h"
#include "benchapi.h"

/// hash 缓冲
#define MAX_HASH_BUF_LEN 4096
char g_hash_buf[MAX_HASH_BUF_LEN];
uint8_t g_md5_sum[32 + 1];

int atoi_safe(const char *nptr) {
	if (nptr == NULL) {
		return 0;
	}

	return atoi(nptr);
}

bool split_json(std::string s, std::vector<std::string>* ret) {
	std::string delim = ",";
	if (s.substr(0, 1) != "{" || s.substr(s.size() - 1, 1) != "}") {
		ERROR_LOG("is not a json string , s is %s", s.c_str());
		return false;
	}
	s = s.substr(1, s.size() - 2);
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos) {
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0) {
		ret->push_back(s.substr(last, index - last));
	}
	return true;
}

//注意：当字符串为空时，也会返回一个空字符串
bool split(std::string s, std::string delim, std::vector<std::string>* ret) {
	size_t last = 0;
	size_t index = s.find(delim, last);
	while (index != std::string::npos) {
		ret->push_back(s.substr(last, index - last));
		last = index + delim.length();
		index = s.find(delim, last);
	}
	if (index - last > 0) {
		ret->push_back(s.substr(last, index - last));
	}
	return true;
}

bool get_json_int(std::vector<std::string> ret, const char * data,
		int * result) {
	//按照  ":  分隔  时间中包含  :
	std::string split_str = "\":";
	std::vector<std::string>::iterator it;
	for (it = ret.begin(); it != ret.end(); it++) {
		std::string key = *it;
		std::vector<std::string> ret_2;
		split(key, split_str, &ret_2);
		//键值对
		if (ret_2.size() != 2) {
			ERROR_LOG("data is %s", data);
			ERROR_LOG("key spilt by '\":',size is not 2 , key is %s,size is %d",
					key.c_str(), ret_2.size());
			return false;
		}
		std::string ret_2_0 = ret_2[0];
		//去除左边引号，右边引号作为分隔字符串已被去除
		if (ret_2_0.size() <= 2) {
			ERROR_LOG("data is %s", data);
			ERROR_LOG(" size is too small  , ret_2_0 is %s", ret_2_0.c_str());
			return false;
		}
		std::string ret_2_0_final = ret_2_0.substr(1, ret_2_0.size() - 1);
		std::string data_key = data;
		if (ret_2_0_final == data_key) {
			*result = atoi_safe(ret_2[1].c_str());
			return true;
		}
	}
	ERROR_LOG("not find this data,data is %s ", data);
	return false;
}

bool get_json_string(std::vector<std::string> ret, const char * data,
		std::string * result) {
	//按照  ":  分隔  时间中包含  :
	std::string split_str = "\":";
	std::vector<std::string>::iterator it;
	for (it = ret.begin(); it != ret.end(); it++) {
		std::string key = *it;
		std::vector<std::string> ret_2;
		split(key, split_str, &ret_2);
		//键值对
		if (ret_2.size() != 2) {
			ERROR_LOG("data is %s", data);
			ERROR_LOG("key spilt by '\":',size is not 2 , key is %s,size is %d",
					key.c_str(), ret_2.size());
			return false;
		}
		std::string ret_2_0 = ret_2[0];
		//去除左边引号，右边引号作为分隔字符串已被去除
		if (ret_2_0.size() <= 2) {
			ERROR_LOG("data is %s", data);
			ERROR_LOG(" size is too small  , ret_2_0 is %s", ret_2_0.c_str());
			return false;
		}
		std::string ret_2_0_final = ret_2_0.substr(1, ret_2_0.size() - 1);
		std::string data_key = data;
		if (ret_2_0_final == data_key) {
			std::string ret_2_1 = ret_2[1];
			//去除引号
			if (ret_2_1.size() <= 2) {
				ERROR_LOG("data is %s", data);
				ERROR_LOG(" size is too small  , ret_2_1 is %s",
						ret_2_1.c_str());
				return false;
			}
			*result = ret_2_1.substr(1, ret_2_1.size() - 2);
			return true;
		}
	}
	ERROR_LOG("not find this data,data is %s ", data);
	return false;
}

const char *trim_str(char *str) {
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
		unsigned char *p_md5_buffer_32, int is_upper) {
	const char *lower_set = "0123456789abcdef";
	const char *upper_set = "0123456789ABCDEF";

	if (is_upper) {
		for (int i = 0; i < 16; ++i) {
			p_md5_buffer_32[2 * i] =
					upper_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
			p_md5_buffer_32[2 * i + 1] =
					upper_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
		}
	} else {
		for (int i = 0; i < 16; ++i) {
			p_md5_buffer_32[2 * i] =
					lower_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
			p_md5_buffer_32[2 * i + 1] =
					lower_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
		}
	}

	return;
}

char get_hex_asc_lower(int i) {
	if (i < 0 || i > 15)
		return ' ';
	if (i < 10)
		return ('0' + i);
	if (i >= 10)
		return ('a' + i - 10);

	return ' ';
}

int hex2asc_lower(uint8_t* dst, size_t len, const uint8_t* src) {
	for (size_t i = 0; i < len; i++) {
		uint8_t tmp = (i % 2 == 0 ? src[i / 2] >> 4 : src[i / 2] & 0x0F);
		dst[i] = get_hex_asc_lower(tmp);
	}

	return 0;
}

const uint8_t* md5_sum_32(const uint8_t* pkg, size_t pkg_len) {
	uint8_t s_raw_md5_sum[16];

	MD5(pkg, pkg_len, s_raw_md5_sum);
	hex2asc_lower(g_md5_sum, 32, s_raw_md5_sum);

	/// 保证字符串正常结束，确保安全
	g_md5_sum[32] = '\0';

	return g_md5_sum;
}

int fill_chnlhash32(chnlhash32_vfy_header_t* vfh, uint16_t chnl_id,
		const char* key, const char* data, int data_len) {
	vfh->chnl_id = chnl_id;

	int hash_body_len = sprintf(g_hash_buf,
			"channelId=%d&securityCode=%s&data=", vfh->chnl_id, key);
	if (hash_body_len <= 0) {
		ERROR_LOG("Verify hash: sprintf error.");
		return -1;
	}

	memcpy(g_hash_buf + hash_body_len, data, data_len);
	hash_body_len += data_len;
	memcpy(vfh->vfy_code, md5_sum_32((uint8_t*) g_hash_buf, hash_body_len), 32);

	return 0;
}

int verify_chnlhash32(const chnlhash32_vfy_header_t* vfh, const char* key,
		const char* data, int data_len) {
	int hash_body_len = sprintf(g_hash_buf,
			"channelId=%d&securityCode=%s&data=", vfh->chnl_id, key);
	if (hash_body_len <= 0) {
		ERROR_LOG("Verify hash: sprintf error.");
		return -1;
	}

	memcpy(g_hash_buf + hash_body_len, data, data_len);
	hash_body_len += data_len;

	if (memcmp(vfh->vfy_code, md5_sum_32((uint8_t*) g_hash_buf, hash_body_len),
			32) == 0)
		return 0;

	return -1;
}
