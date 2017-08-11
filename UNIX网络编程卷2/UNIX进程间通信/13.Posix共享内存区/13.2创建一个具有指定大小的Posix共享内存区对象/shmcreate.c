#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, fd, flags;
	char	*ptr;
	off_t	length;

	flags = O_RDWR | O_CREAT; //模式定义成读写,没有则创建
	while ( (c = Getopt(argc, argv, "e")) != -1) { //格式化命令行参数
		switch (c) {
		case 'e': //如果有-e选项就加上排他标志
			flags |= O_EXCL;
			break;
		}
	}
	if (optind != argc - 2) //-e后面的参数应该是倒数第二个参数,此参数代表众所周知的IPC名
		err_quit("usage: shmcreate [ -e ] <name> <length>");
	length = atoi(argv[optind + 1]); //取出变量中的共享内存区长度参数

	fd = Shm_open(argv[optind], flags, FILE_MODE); //按指定模式打开或创建共享内存区对象
	Ftruncate(fd, length); //根据命令行参数调整共享内存区对象长度

	ptr = Mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //使用mmap进行映射

	exit(0);
}
