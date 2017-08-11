#include	"svmsg.h"

void	client(int, int);

int
main(int argc, char **argv)
{
	int		msqid; //消息队列id


		/* 4server must create the queue */
	msqid = Msgget(MQ_KEY1, 0); //打开MQ_KEY1所代表的消息队列

	client(msqid, msqid);	/* same queue for both directions 客户端的行为都在这个函数中执行*/

	exit(0);
}
