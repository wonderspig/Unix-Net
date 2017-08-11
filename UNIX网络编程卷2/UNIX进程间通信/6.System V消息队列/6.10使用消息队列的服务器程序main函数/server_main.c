#include	"svmsg.h"

void	server(int, int);

int
main(int argc, char **argv)
{
	int		readid, writeid; //消息队列MQ_KEY1用来读,消息队列MQ_KEY2用来写

	readid = Msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT); //创建MQ_KEY1的消息队列用来读，已存在则打开
	writeid = Msgget(MQ_KEY2, SVMSG_MODE | IPC_CREAT); //创建MQ_KEY2的消息队列用来写,已存在则打开

	server(readid, writeid); //服务器的具体工作在此函数中处理,使用4-30的版本

	exit(0);
}
