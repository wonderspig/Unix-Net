#include	"unpipc.h"

struct shmstruct {	/* struct stored in shared memory */
  int	count;
};
sem_t	*mutex;		/* pointer to named semaphore */

int
main(int argc, char **argv)
{
	int		fd;
	struct shmstruct	*ptr;

	if (argc != 3) //如果命令行参数不等于2,则出错
		err_quit("usage: server1 <shmname> <semname>");

	shm_unlink(Px_ipc_name(argv[1]));		/* OK if this fails 先删除共享内存区(防止已存在)*/
		/* 4create shm, set its size, map it, close descriptor */
	fd = Shm_open(Px_ipc_name(argv[1]), O_RDWR | O_CREAT | O_EXCL, FILE_MODE); //打开共享内存区
	Ftruncate(fd, sizeof(struct shmstruct)); //调整共享内存区大小
	ptr = Mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射共享内存区
	Close(fd); //关闭文件,不影响已经映射的共享内存区

	sem_unlink(Px_ipc_name(argv[2]));		/* OK if this fails 先删除信号量(防止已存在)*/
	mutex = Sem_open(Px_ipc_name(argv[2]), O_CREAT | O_EXCL, FILE_MODE, 1); //打开创建信号量
	Sem_close(mutex);//关闭文件,不影响已经打开的信号量

	exit(0);
}
