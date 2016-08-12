#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PACK_LEN (18)

typedef struct {
    uint32_t package_len;
    uint32_t seq_num;
    uint16_t command_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) test_proto_bean_t;


int main(int argc,char** argv)
{
	if(argc<3)
	{
		printf("usage:send_msg_sync_test mq_name user_id  repeatcount\n",argc);
		return -1;
	}

	const char* mq_name=argv[1];
	int user_id = atoi(argv[2]);
	int repeatcount = atoi(argv[3]); // 

	printf("mq_name:%s,user_id:%d,repeatcount\n",mq_name,user_id,repeatcount);

	int oflag=O_WRONLY;
	
	mqd_t mqd = mq_open(mq_name,oflag);
	if(mqd==-1)
	{
		printf("mq_open() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
		return -1;
	}

	printf("mq_open() success.mqd:%d\n",(int)mqd);

	for(int i=0;i<repeatcount;i++)
	{
		char data[PACK_LEN] = {0};
		test_proto_bean_t* proto = (test_proto_bean_t*)data;		
		proto->package_len=PACK_LEN;
		proto->command_id=1002;
		proto->user_id=user_id;

		if(mq_send(mqd,data,sizeof(data),1)==-1)
		{
			printf("mq_send() error.errno:%d\n",errno);
			printf("errinfo:%s\n",strerror(errno) );
		}
		else
		{
			printf("%d \t",i);
		}
	}//for

	mq_close(mqd);
	return 0;
}
