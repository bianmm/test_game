#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc,char** argv)
{
	const char* mq_name;

	if(argc<4)
	{
		printf("usage:mq_create2 name msg_max msgsize_max");
		return -1;
	}

	mq_name=argv[1];
	int msg_max = atoi(argv[2]);
	int msgsize_max = atoi(argv[3]);

	int oflag;
	oflag=O_RDWR|O_CREAT;
	
	mode_t old=umask(0000);	
	printf("old mask:%o\n",old);

	mq_attr attr;
	memset(&attr,0,sizeof(attr));
	attr.mq_flags = 0;
	attr.mq_maxmsg = msg_max;
	attr.mq_msgsize = msgsize_max;
	attr.mq_curmsgs = 0;

	printf("try to create or open mq:%s,msg_max:%d,msgsize_max:%d\n",mq_name,msg_max,msgsize_max);

	mqd_t mqd = mq_open(mq_name,oflag,0744,&attr);
	if(mqd==-1)
	{
		printf("mq_open() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
	}
	else
	{
		printf("mq_open() success.mqd:%d\n",(int)mqd);
	}

	return 0;
}
