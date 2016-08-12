#ifndef UTIL_H
#define UTIL_H

#include <time.h>
#include <stdint.h>
#include <vector>

void asc_to_hex_lowercase(char* sz_dst, char* sz_src,int len);

int print_in_hex(const char* src,int len);

int parse_str_time(const char* str_time,time_t& time);

int check_client_ip(std::vector<uint32_t>& allow_ips, uint32_t ip);

int string_char_replace(char* str, char src, char des);

int millisleep(int milli_seconds);

#endif
