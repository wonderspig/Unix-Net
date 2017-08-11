#include	"unpipc.h"

#define	MAXMSG	(8192 + sizeof(long))

int
main(int argc, char **argv)
{
	int		mqid;
	ssize_t	n;
	struct msgbuf	*buff;

	if (argc != 2) //如果命令行参数不等于1,则报错
		err_quit("usage: msgrcvid <mqid>");
	mqid = atoi(argv[1]); //把命令行的第一个参数转换为数值形式,第一个参数为消息队列的标识符

	buff = Malloc(MAXMSG); //分配接受消息队列消息的缓存

	n = Msgrcv(mqid, buff, MAXMSG, 0, 0); //用命令行第一个参数传进来的标识符所代表的消息队列中读取数据
	printf("read %d bytes, type = %ld\n", n, buff->mtype); //打印读取出来的消息队列的信息

	exit(0);
}
