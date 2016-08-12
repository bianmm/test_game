#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PACK_LEN (8192)
char data[PACK_LEN];

typedef struct {
	uint32_t pkg_len;
	uint16_t version;
	uint32_t disp_key;
	uint32_t group_id;
} __attribute__((packed)) sync_msg_header_t;

typedef struct {
    uint32_t package_len;
    uint32_t seq_num;
    uint16_t command_id;
    uint32_t status_code;
    uint32_t user_id;
	uint32_t flag;
	uint32_t months; //用户目前总包月月数
	uint32_t vip_end_time;
	uint32_t is_auto_charge;
	uint32_t vip_begin_time;
	uint32_t last_charge_chnl_id;
} __attribute__((packed)) vip_proto_pkg_t;


int main(int argc,char** argv)
{
	if(argc<6)
	{
		printf("usage:send_msg mq_name repeat_count pack_cnt disp_key group_id\n",argc);
		return -1;
	}

	const char* mq_name=argv[1];
	int repeat_count = atoi(argv[2]);
	int pack_cnt = atoi(argv[3]);
	int disp_key = atoi(argv[4]);
	int group_id = atoi(argv[5]);

	printf("mq_name:%s,repeat_count:%d,pack_cnt:%d,disp_key:%d,group_id:%d\n",mq_name,repeat_count,pack_cnt,disp_key,group_id);

	if (pack_cnt * (sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t)) > PACK_LEN)
	{
		printf("pack_len %u not engough, need %u\n", PACK_LEN, pack_cnt * (sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t)));
		return -1;
	}

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
		for (int j=0; j<pack_cnt; j++)
		{
			sync_msg_header_t* h = (sync_msg_header_t*)(data + j * (sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t)));
			vip_proto_pkg_t* pkg = (vip_proto_pkg_t*)(data + j * (sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t)) + sizeof(sync_msg_header_t));
			h->pkg_len = sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t);
			h->disp_key = disp_key;
			h->group_id = group_id;
			pkg->package_len=sizeof(vip_proto_pkg_t) + 12;
			pkg->command_id=0x196E;
			pkg->user_id = 50077;
			pkg->flag = 3;
			pkg->months = 123;
			pkg->vip_end_time = 321;
			pkg->is_auto_charge = 7;
			pkg->vip_begin_time = 456;
			pkg->last_charge_chnl_id = 67;
		}

		if(mq_send(mqd,data,pack_cnt * (sizeof(sync_msg_header_t) + sizeof(vip_proto_pkg_t)),1)==-1)
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
