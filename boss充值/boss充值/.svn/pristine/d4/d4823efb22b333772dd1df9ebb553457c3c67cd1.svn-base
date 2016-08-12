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

	if(argc<2)
	{
		printf("usage:get_mq_attr mq_name\n");
		return -1;
	}

	mq_name=argv[1];
	mqd_t mqd = mq_open(mq_name,0);
	if(mqd==-1)
	{
		printf("mq_open() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
		return -1;
	}

	mq_attr attr;
	memset(&attr,0,sizeof(attr));

	if(mq_getattr(mqd,&attr)==0)
	{
		printf("%ld", attr.mq_curmsgs);
	}
	else
	{
		printf("mq_getattr() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
		return -1;
	}

	return 0;
}
