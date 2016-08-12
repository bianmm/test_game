#include "util.h"

/**
 * @brief 检查目标IP是否在允许IP地址列表中
 *
 * @return return 0 if ip is allowed, -1 if not allowed
 */
int check_ip(std::vector<uint32_t>& allow_ips, uint32_t ip)
{
	int size = allow_ips.size();
	if (size == 0)
	{
		return -1;
	}

	for (int i=0; i<size; i++)
	{
		if (allow_ips[i] == ip)
		{
			//found
			return 0;
		}
	}

	//not found
	return -1;
}

