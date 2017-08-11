#include	"unpipc.h"

#define	MAXMSG	(8192 + sizeof(long))

int
main(int argc, char **argv)
{
	int		c, flag, mqid;
	long	type;
	ssize_t	n;
	struct msgbuf	*buff;

	type = flag = 0; //把msgrcv的第三第四个参数设置为0
	while ( (c = Getopt(argc, argv, "nt:")) != -1) { //获取命令行参数
		switch (c) {
		case 'n': //如果命令行参数为-n
			flag |= IPC_NOWAIT; //则把msgrcv的第四个参数或上IPC_NOWAIT位
			break;

		case 't': //如果命令行参数为-t
			type = atol(optarg); //则取-n后面的详细属性值,转换成数值形式,赋值给type
			break;
		}
	}
	if (optind != argc - 1) //如果命令行已经是最后一个参数了,则报错
		err_quit("usage: msgrcv [ -n ] [ -t type ] <pathname>");

	mqid = Msgget(Ftok(argv[optind], 0), MSG_R); //用下一个命令行参数打开一个消息队列

	buff = Malloc(MAXMSG); //分配存放取出消息队列的缓冲区

	n = Msgrcv(mqid, buff, MAXMSG, type, flag); //从消息队列中取出数据,type和flag已经根据命令行参数设置
	printf("read %d bytes, type = %ld\n", n, buff->mtype); //打印取出的消息队列中的数据

	exit(0);
}
