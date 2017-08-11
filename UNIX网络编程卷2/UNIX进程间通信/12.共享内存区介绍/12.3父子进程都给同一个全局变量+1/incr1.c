#include	"unpipc.h"

#define	SEM_NAME	"mysem"

int		count = 0;

int
main(int argc, char **argv)
{
	int		i, nloop;
	sem_t	*mutex;

	if (argc != 2) //如果命令行参数不等于2,则出错
		err_quit("usage: incr1 <#loops>");
	nloop = atoi(argv[1]); //命令行第一个参数为循环次数(信号量的初始值)

		/* 4create, initialize, and unlink semaphore */
	mutex = Sem_open(Px_ipc_name(SEM_NAME), O_CREAT | O_EXCL, FILE_MODE, 1); //排他的创建一个信号量(二值信号量)
	Sem_unlink(Px_ipc_name(SEM_NAME)); //删除这个信号量(文件删除后,并不影响内存中信号量的使用)

	setbuf(stdout, NULL);	/* stdout is unbuffered 把标准输出设置成无缓冲的 */
	//创建子进程
	if (Fork() == 0) {		/* child 在子进程中 */
		for (i = 0; i < nloop; i++) { //循环nloop,自增count
			Sem_wait(mutex);
			printf("child: %d\n", count++);
			Sem_post(mutex);
		}
		exit(0);
	}

		/* 4parent 父进程中 */
	for (i = 0; i < nloop; i++) { //循环nloop,自增count
		Sem_wait(mutex);
		printf("parent: %d\n", count++);
		Sem_post(mutex);
	}
	exit(0);
}
