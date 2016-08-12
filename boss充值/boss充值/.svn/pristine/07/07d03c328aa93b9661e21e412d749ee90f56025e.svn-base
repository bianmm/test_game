#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PACK_LEN (18+40)

typedef struct {
    uint32_t package_len;
    uint32_t seq_num;
    uint16_t command_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) vip_proto_header_t;


int main(int argc,char** argv)
{
	if(argc<3)
	{
		printf("usage:send_msg mq_name repeat_count\n",argc);
		return -1;
	}

	const char* mq_name=argv[1];
	int repeat_count = atoi(argv[2]);

	printf("mq_name:%s,repeat_count:%d\n",mq_name,repeat_count);

	int oflag=O_WRONLY;
	
	mqd_t mqd = mq_open(mq_name,oflag);
	if(mqd==-1)
	{
		printf("mq_open() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
		return -1;
	}

	printf("mq_open() success.mqd:%d\n",(int)mqd);

	for(int i=0;i<repeat_count;i++)
	{

	char data[PACK_LEN];
	vip_proto_header_t* header = (vip_proto_header_t*)data;		
	header->package_len=PACK_LEN;
	header->command_id=0x2007;

	if(mq_send(mqd,data,sizeof(data),1)==-1)
	{
		printf("mq_send() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
	}
	else
	{
		printf("%d\t",i);
	}
		
	}//for

	mq_close(mqd);
	return 0;
}
