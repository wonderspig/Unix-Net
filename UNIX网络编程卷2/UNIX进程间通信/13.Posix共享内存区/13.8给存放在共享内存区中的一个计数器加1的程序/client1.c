#include	"unpipc.h"

struct shmstruct {	/* struct stored in shared memory */
  int	count;
};
sem_t	*mutex;		/* pointer to named semaphore */

int
main(int argc, char **argv)
{
	int		fd, i, nloop;
	pid_t	pid;
	struct shmstruct	*ptr;

	if (argc != 4) //如果命令行参数不等于3,则出错
		err_quit("usage: client1 <shmname> <semname> <#loops>");
	nloop = atoi(argv[3]); //命令行最后一个参数是循环递增的次数

	fd = Shm_open(Px_ipc_name(argv[1]), O_RDWR, FILE_MODE); //打开命令行参数指定的共享内存区
	ptr = Mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射该共享内存区
	Close(fd); //关闭该对象不影响已经映射的共享内存区

	mutex = Sem_open(Px_ipc_name(argv[2]), 0); //打开信号量

	pid = getpid(); //获得本进程pid
	for (i = 0; i < nloop; i++) { //在信号量的保护下进行数值的递增
		Sem_wait(mutex);
		printf("pid %ld: %d\n", (long) pid, ptr->count++);
		Sem_post(mutex);
	}
	exit(0);
}
