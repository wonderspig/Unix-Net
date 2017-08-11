#include	"unpipc.h"

int
main(int argc, char **argv)
{
	if (argc != 2) //如果参数不等于1,则出错
		err_quit("usage: semunlink <name>");

	Sem_unlink(argv[1]); //删除命令行第一个参数锁代表的信号量

	exit(0);
}
