#include	"svmsg.h"

void	client(int, int);

int
main(int argc, char **argv)
{
	int		readid, writeid; //消息队列MQ_KEY1用来读,消息队列MQ_KEY2用来写

		/* 4assumes server has created the queues */
	writeid = Msgget(MQ_KEY1, 0); //打开MQ_KEY1消息队列
	readid = Msgget(MQ_KEY2, 0); //打开MQ_KEY2消息队列

	client(readid, writeid); //client函数使用了4-29中的版本,客户端的工作全在这个函数中执行

		/* 4now we can delete the queues */
	Msgctl(readid, IPC_RMID, NULL); //删除读消息队列
	Msgctl(writeid, IPC_RMID, NULL); //删除写消息队列

	exit(0);
}
