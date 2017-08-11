#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		mqid;
	size_t	len;  //命令行第三个参数是长度,保存在此
	long	type; //命令行第四个参数是类型,保存在此
	struct msgbuf	*ptr;

	if (argc != 4) //如果参数不等于3,则报错退出
		err_quit("usage: msgsnd <pathname> <#bytes> <type>");
	len = atoi(argv[2]); //命令行第三个参数是长度
	type = atoi(argv[3]); //命令行第四个参数是类型

	//获得一个已存在的消息队列(如何获得已存在的消息队列的关键在于MSG_W)
	//用命令行的第一个参数提供的路径生成KEY, 权限位是MSG_W
	mqid = Msgget(Ftok(argv[1], 0), MSG_W);

	ptr = Calloc(sizeof(long) + len, sizeof(char)); //分配数据缓冲区, sizeof(long) + len个char的大小
	ptr->mtype = type; //ptr的类型成员为type

	//往消息队列中放入数据,放不下函数将会阻塞
	//len的长度不包括开始的ptr->mtype成员, 只包括纯粹的数据成员
	Msgsnd(mqid, ptr, len, 0);

	exit(0);
}
