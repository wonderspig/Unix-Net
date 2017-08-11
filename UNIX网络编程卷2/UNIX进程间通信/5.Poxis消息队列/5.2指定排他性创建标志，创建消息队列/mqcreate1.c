#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;

	flags = O_RDWR | O_CREAT; //指定标志为读写,创建(不排他)
	while ( (c = Getopt(argc, argv, "e")) != -1) { //Getopt此函数为标准化命令行参数,表示带e参数,且e参数后面不跟属性
		switch (c) {
		case 'e':
			flags |= O_EXCL; //如果有-e参数,则加上排他属性
			break;
		}
	}
	if (optind != argc - 1) //全局变量optind为:下一个待处理参数的下标
		err_quit("usage: mqcreate [ -e ] <name>");

	mqd = Mq_open(argv[optind], flags, FILE_MODE, NULL); //根据设置好的模式打开消息队列

	Mq_close(mqd); //关闭该消息队列(消息队列是随内核的,所以只要没有删除,消息队列随内核存在永远存在)
	exit(0);
}
