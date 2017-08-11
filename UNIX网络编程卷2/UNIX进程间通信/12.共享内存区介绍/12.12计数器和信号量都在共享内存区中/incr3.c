#include	"unpipc.h"

struct shared {
  sem_t	mutex;		/* the mutex: a Posix memory-based semaphore */
  int	count;		/* and the counter */
} shared;

int
main(int argc, char **argv)
{
	int		fd, i, nloop;
	struct shared 	*ptr;

	if (argc != 3) //如果命令行参数不等于2,则出错
		err_quit("usage: incr3 <pathname> <#loops>");
	nloop = atoi(argv[2]); //第二个参数是循环递增的次数

		/* 4open file, initialize to 0, map into memory */
	fd = Open(argv[1], O_RDWR | O_CREAT, FILE_MODE); //创建第一个参数所指定的路径文件
	Write(fd, &shared, sizeof(struct shared)); //往这个空文件中写入一个结构体(调整文件长度为这个结构体的大小)
	ptr = Mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射文件到内存
	Close(fd); //关闭文件,并影响已映射的指针

		/* 4initialize semaphore that is shared between processes */
	Sem_init(&ptr->mutex, 1, 1); //初始化信号量,属性为进程间共享(二值信号量)

	setbuf(stdout, NULL);	/* stdout is unbuffered 标准输出设置为无缓冲 */
	//创建子进程(内存映射区会继承下来)
	if (Fork() == 0) {		/* child 在子进程中 */
		for (i = 0; i < nloop; i++) { //循环递增共享内存区中的ptr->count++
			Sem_wait(&ptr->mutex);
			printf("child: %d\n", ptr->count++);
			Sem_post(&ptr->mutex);
		}
		exit(0);
	}

		/* 4parent 在父进程中 */
	for (i = 0; i < nloop; i++) { //循环递增共享内存区中的ptr->count++
		Sem_wait(&ptr->mutex);
		printf("parent: %d\n", ptr->count++);
		Sem_post(&ptr->mutex);
	}
	exit(0);
}
