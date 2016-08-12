#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <vector>

int check_ip(std::vector<uint32_t>& allow_ips, uint32_t ip);

#endif
