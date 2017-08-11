#include	"svmsg.h"

void	server(int, int);

int
main(int argc, char **argv)
{
	int		msqid; //用来保存消息队列的id

	msqid = Msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT); //使用MQ_KEY1来创建消息队列,已存在就打开

	server(msqid, msqid);	/* same queue for both directions 服务器的行为全在这个函数中实现*/

	exit(0);
}
