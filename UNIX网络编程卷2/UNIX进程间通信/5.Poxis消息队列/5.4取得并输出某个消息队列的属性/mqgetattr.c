#include	"unpipc.h"

int
main(int argc, char **argv)
{
	mqd_t	mqd;
	struct mq_attr	attr;

	if (argc != 2) //如果命令行参数不为1,则出错
		err_quit("usage: mqgetattr <name>");

	mqd = Mq_open(argv[1], O_RDONLY); //以读的方式打开命令行参数所指定的消息队列

	Mq_getattr(mqd, &attr); //获得这个消息队列的属性,填写入attr
	printf("max #msgs = %ld, max #bytes/msg = %ld, "
		   "#currently on queue = %ld\n",
		   attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs); //打印这个消息队列的属性

	Mq_close(mqd); //关闭这个消息队列
	exit(0);
}
