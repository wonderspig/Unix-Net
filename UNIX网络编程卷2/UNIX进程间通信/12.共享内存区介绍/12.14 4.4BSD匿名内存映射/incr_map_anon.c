#include	"unpipc.h"

#define	SEM_NAME	"mysem"

/* include diff */
int
main(int argc, char **argv)
{
	int		i, nloop;
	int		*ptr;
	sem_t	*mutex;

	if (argc != 2) //如果参数不等于1,则出错
		err_quit("usage: incr_map_anon <#loops>");
	nloop = atoi(argv[1]); //第一个餐素是递增的循环次数

		/* 4map into memory */
	ptr = Mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			   MAP_SHARED | MAP_ANON, -1, 0); //匿名映射的BSD实现为:加上标志MAP_ANON以及文件描述符为-1
/* end diff */

		/* 4create, initialize, and unlink semaphore */
	mutex = Sem_open(Px_ipc_name(SEM_NAME), O_CREAT | O_EXCL, FILE_MODE, 1);
	Sem_unlink(Px_ipc_name(SEM_NAME));

	setbuf(stdout, NULL);	/* stdout is unbuffered */
	if (Fork() == 0) {		/* child */
		for (i = 0; i < nloop; i++) {
			Sem_wait(mutex);
			printf("child: %d\n", (*ptr)++);
			Sem_post(mutex);
		}
		exit(0);
	}

		/* 4parent */
	for (i = 0; i < nloop; i++) {
		Sem_wait(mutex);
		printf("parent: %d\n", (*ptr)++);
		Sem_post(mutex);
	}
	exit(0);
}
