#include <string.h>
#include <time.h>

#include "util.h"
#include "log.h"

//-----------------------------------//
//-----------------------------------//

void asc_to_hex_lowercase(char* sz_dst, char* sz_src,int len)
{
    int hex = 0;
    int index = 0;
    int dest_index = 0;

    for(index = 0; index < len; index++)
    {
        hex = ((unsigned char)sz_src[index]) >> 4;
        dest_index = index * 2;
        sz_dst[dest_index] = hex < 10 ? '0' + hex : 'a' - 10 + hex ;

        hex = ((unsigned char)sz_src[index]) & 0x0F;
        sz_dst[dest_index + 1] = hex < 10 ? '0' + hex : 'a'- 10 + hex ;
    }

    sz_dst[len * 2] = 0;
}

//-----------------------------------//
//-----------------------------------//

int print_in_hex(const char* src,int len)
{
	char buffer[1024*8] = { 0 };
	asc_to_hex_lowercase(buffer, (char*)src, len);
	TRACE_LOG("[%s]", buffer);

	return 0;
}

//-----------------------------------//
//-----------------------------------//

int parse_str_time(const char* str_time,time_t& time)
{
	if (str_time == NULL || strlen(str_time) == 0 )
	{
		return -1;
	}

	const char* format="%Y-%m-%d %H:%M:%S";
	struct tm tm;
	char* result=NULL;

	result=strptime(str_time, format, &tm);
	if(result==NULL)
	{
		return -1;
	}

	time=mktime(&tm);
	if(time==-1)
	{
		return -1;
	}

	return 0;
}

//-----------------------------------//
//-----------------------------------//

/**
 *
 * @return return 0 if ip is allowed, -1 if not allowed
 */
int check_client_ip(std::vector<uint32_t>& allow_ips, uint32_t ip)
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

int string_char_replace(char* str, char src, char des)
{
	if (!str)
	{
		return 0;
	}

	for (int i=0; i<(int)strlen(str); i++)
	{
		if (str[i] == src)
		{
			str[i] = des;
		}
	}

	return 0;
}


/**
 * 睡眠指定毫秒数
 */
int millisleep(int milli_seconds)
{
	timespec time_interval;
	time_interval.tv_sec = 0;
	time_interval.tv_nsec = 1000000*milli_seconds;

	return nanosleep(&time_interval, NULL);
}
