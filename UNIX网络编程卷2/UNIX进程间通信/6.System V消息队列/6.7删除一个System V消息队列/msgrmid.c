#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		mqid;

	if (argc != 2) //如果命令行参数不是一个,则报错
		err_quit("usage: msgrmid <pathname>");

	mqid = Msgget(Ftok(argv[1], 0), 0); //打开命令行参数所生成KEY所代表的消息队列
	Msgctl(mqid, IPC_RMID, NULL); //使用IPC_RMID命令,删除这个消息队列

	exit(0);
}
