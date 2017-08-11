#include	"unpipc.h"

int
main(int argc, char **argv)
{
	sem_t	*sem;
	int		val;

	if (argc != 2) //命令行参数小于2,则出错
		err_quit("usage: sempost <name>");

	sem = Sem_open(argv[1], 0); //打开这个信号量
	Sem_post(sem); //挂出这个信号量
	Sem_getvalue(sem, &val); //取得这个信号量当前的值,存放在vl中
	printf("value = %d\n", val); //打印信号量当前的值

	exit(0);
}
