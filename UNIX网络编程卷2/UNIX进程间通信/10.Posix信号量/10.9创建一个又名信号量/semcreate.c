#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, flags;
	sem_t	*sem;
	unsigned int	value;

	flags = O_RDWR | O_CREAT; //创建信号量的模式为读写,如果没有这个信号量,则创建
	value = 1; //默认信号量为二值信号量
	while ( (c = Getopt(argc, argv, "ei:")) != -1) { //格式化命令行参数,格式为-e不带属性,－i带属性
		switch (c) {
		case 'e': //如果命令行参数带-e
			flags |= O_EXCL; //则加上拍他标志
			break;

		case 'i': //如果有-i参数
			value = atoi(optarg); //则取-i后面的属性值,并转换成数值格式,存放在value中
			break;
		}
	}
	if (optind != argc - 1) //下一个参数不是命令行的最后一个参数,则出错
		err_quit("usage: semcreate [ -e ] [ -i initalvalue ] <name>");

	sem = Sem_open(argv[optind], flags, FILE_MODE, value); //根据命令行参数参数创建信号量

	Sem_close(sem); //关闭信号量,信号量至少是随内核存在的,关闭它也不会删除信号量
	exit(0);
}
