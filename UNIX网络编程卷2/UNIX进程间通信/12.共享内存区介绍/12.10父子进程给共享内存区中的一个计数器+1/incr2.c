#include	"unpipc.h"

#define	SEM_NAME	"mysem"

int
main(int argc, char **argv)
{
	int		fd, i, nloop, zero = 0;
	int		*ptr;
	sem_t	*mutex;

	if (argc != 3) //如果命令行参数不等于2,则出错
		err_quit("usage: incr2 <pathname> <#loops>");
	nloop = atoi(argv[2]); //第二个参数为递增的循环次数

		/* 4open file, initialize to 0, map into memory */
	fd = Open(argv[1], O_RDWR | O_CREAT, FILE_MODE); //读写创建的模式打开命令行参数1所指定路径的文件
	Write(fd, &zero, sizeof(int)); //像文件中写入一个值为0的整数(调整文件大小)
	ptr = Mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //映射文件到内存区
	Close(fd); //映射完了,可以关闭文件,不会影响已经映射的文件

		/* 4create, initialize, and unlink semaphore */
	mutex = Sem_open(Px_ipc_name(SEM_NAME), O_CREAT | O_EXCL, FILE_MODE, 1); //排他创建一个信号量
	Sem_unlink(Px_ipc_name(SEM_NAME)); //删除这个信号量不影响该信号量在内存中的使用

	setbuf(stdout, NULL);	/* stdout is unbuffered 设置标准输出位无缓冲 */
	//创建子进程
	//信号量是在fork之前创建的,所以父子进程可以直接使用这个信号量来进行同步
	if (Fork() == 0) {		/* child 在子进程中 */
		for (i = 0; i < nloop; i++) {
			Sem_wait(mutex); //获得信号量
			printf("child: %d\n", (*ptr)++); //在共享内存区中的变量自增1
			Sem_post(mutex); //挂出信号量
		}
		exit(0);
	}

		/* 4parent */
	for (i = 0; i < nloop; i++) {
		Sem_wait(mutex); //获得信号量
		printf("parent: %d\n", (*ptr)++); //在共享内存区中的变量自增1
		Sem_post(mutex); //挂出信号量
	}
	exit(0);
}
