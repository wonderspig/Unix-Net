#include	"unpipc.h"

int
main(int argc, char **argv)
{
	sem_t	*sem;
	int		val;

	if (argc != 2) //如果命令行参数不为1,则出错
		err_quit("usage: semgetvalue <name>");

	sem = Sem_open(argv[1], 0); //把命令行参数锁代表的信号量打开
	Sem_getvalue(sem, &val); //获得这个信号量的值
	printf("value = %d\n", val); //打印这个值

	exit(0); //退出程序是会顺便关闭信号量
}
