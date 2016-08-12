#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PACK_LEN (18+4)

typedef struct {
    uint32_t package_len;
    uint32_t seq_num;
    uint16_t command_id;
    uint32_t status_code;
    uint32_t user_id;
    uint32_t bean_num;
} __attribute__((packed)) vip_proto_bean_t;


int main(int argc,char** argv)
{
	if(argc<4)
	{
		printf("usage:send_msg_seer_bean mq_name user_id bean_num repeatcount\n",argc);
		return -1;
	}

	const char* mq_name=argv[1];
	int user_id = atoi(argv[2]);
	int bean_num = atoi(argv[3]); //1 金豆则bean_num =1 
	int repeatcount = atoi(argv[4]); // 

	printf("mq_name:%s,user_id:%d,bean_num,repeatcount\n",mq_name,user_id,bean_num,repeatcount);

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
		char data[PACK_LEN];
		vip_proto_bean_t* proto = (vip_proto_bean_t*)data;		
		proto->package_len=PACK_LEN;
		proto->command_id=0x19a6;
		proto->user_id=user_id;
		proto->bean_num=bean_num*100;

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
