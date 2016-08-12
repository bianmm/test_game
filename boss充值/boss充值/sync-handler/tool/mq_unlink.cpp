#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc,char** argv)
{
	const char* mq_name;
	if(argc>1)
	{
		mq_name = argv[1];	
		printf("remove mq %s\n",mq_name);
	}
	else
	{
		printf("give mq_name!\n");
		return 0;
	}

	mqd_t mqd = mq_unlink(mq_name);
	if(mqd==-1)
	{
		printf("mq_unlink() error.errno:%d\n",errno);
		printf("errinfo:%s\n",strerror(errno) );
	}
	else
	{
		printf("mq_unlink() success.mqd:%d\n",(int)mqd);
	}

	return 0;
}
