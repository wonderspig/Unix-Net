#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;
	ssize_t	n;
	unsigned int	prio;
	void	*buff;
	struct mq_attr	attr;

	flags = O_RDONLY; //标志设置为只读
	while ( (c = Getopt(argc, argv, "n")) != -1) { //标准化命令行参数,只有一个参数-n，没有属性
		switch (c) {
		case 'n': //有-n参数则加上非阻塞属性
			flags |= O_NONBLOCK;
			break;
		}
	}
	if (optind != argc - 1) //optind为下一个参数的下标索引
		err_quit("usage: mqreceive [ -n ] <name>");

	mqd = Mq_open(argv[optind], flags); //打开命令行参数所表示的消息队列
	Mq_getattr(mqd, &attr); //获得这个消息队列的属性

	buff = Malloc(attr.mq_msgsize); //安消息队列属性中的每个消息大小分配一个动态穿冲去,存放消息
	//从消息队列中取消息
	n = Mq_receive(mqd, buff, attr.mq_msgsize, &prio);
	printf("read %ld bytes, priority = %u\n", (long) n, prio); //打印这个消息的长度,和优先级

	exit(0);
}
