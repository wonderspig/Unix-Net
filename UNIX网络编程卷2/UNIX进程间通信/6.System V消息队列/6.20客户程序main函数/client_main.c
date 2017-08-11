#include	"svmsg.h"

void	client(int, int);

int
main(int argc, char **argv)
{
	int		readid, writeid; //读的消息队列id,和写消息队列id

		/* 4server must create its well-known queue */
	writeid = Msgget(MQ_KEY1, 0); //打开MQ_KEY1所代表的消息队列
		/* 4we create our own private queue */
	readid = Msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT); //创建客户进程私有的消息队列,用来读服务器应答

	client(readid, writeid); //客户程序的行为都在此函数中执行

		/* 4and delete our private queue */
	Msgctl(readid, IPC_RMID, NULL); //删除私有的用来读服务器应答的消息队列

	exit(0);
}
