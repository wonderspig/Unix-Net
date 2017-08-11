#include	"unpipc.h"

int
main(int argc, char **argv)
{
	sem_t	*sem;
	int		val;

	if (argc != 2) //参数不等于2,则出错
		err_quit("usage: semwait <name>");

	sem = Sem_open(argv[1], 0); //打开信号量
	Sem_wait(sem); //等待这个信号量
	Sem_getvalue(sem, &val); //获得这个信号量当前的值
	printf("pid %ld has semaphore, value = %d\n", (long) getpid(), val); //打印这个信号量当前的值

	pause();	/* blocks until killed 休眠等待信号 */
	exit(0);
}
