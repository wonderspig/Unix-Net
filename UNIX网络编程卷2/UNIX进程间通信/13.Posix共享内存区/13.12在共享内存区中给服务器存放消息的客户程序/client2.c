#include	"cliserv2.h"

int
main(int argc, char **argv)
{
	int		fd, i, nloop, nusec;
	pid_t	pid;
	char	mesg[MESGSIZE];
	long	offset;
	struct shmstruct	*ptr;

	if (argc != 4) //命令行参数,第一个是共享内存区名,第二个是循环递增的次数,第三格式每次循环的间隔时间
		err_quit("usage: client2 <name> <#loops> <#usec>");
	nloop = atoi(argv[2]); //把参数2,循环递增的次数,转换成数值
	nusec = atoi(argv[3]); //把参数3,每次循环间隔的时间,转换成数值

		/* 4open and map shared memory that server must create */
	fd = Shm_open(Px_ipc_name(argv[1]), O_RDWR, FILE_MODE); //打开IPC共享内存区
	ptr = Mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射共享内存
	Close(fd); //关闭IPC对象,不影响已经映射的共享内存

	pid = getpid(); //获得本进程pid
	for (i = 0; i < nloop; i++) { //开始循环,次数为命令行参数决定的
		Sleep_us(nusec); //睡nusec微妙
		snprintf(mesg, MESGSIZE, "pid %ld: message %d", (long) pid, i); //把本进程的pid和消息数值打印到mesg中

		if (sem_trywait(&ptr->nempty) == -1) { //非阻塞的获取空槽位信号量
			if (errno == EAGAIN) { //这个错误表示现在空槽位信号量已经没有了
				Sem_wait(&ptr->noverflowmutex); //获取溢出计数器的二值信号量
				ptr->noverflow++; //递增计数器
				Sem_post(&ptr->noverflowmutex); //挂出溢出计数器的二值信号量
				continue; //不执行后面的代码直接继续循环
			} else
				err_sys("sem_trywait error");
		}
		Sem_wait(&ptr->mutex); //加锁(二值信号量)
		offset = ptr->msgoff[ptr->nput]; //通过"下一次存放下标"这个成员,获取存放在数组中的偏移量
		if (++(ptr->nput) >= NMESG) //下标大于数组的最大下标,则归0(环形缓冲区)
			ptr->nput = 0;		/* circular buffer */
		Sem_post(&ptr->mutex); //解锁(二值信号量)
		strcpy(&ptr->msgdata[offset], mesg); //把格式化好的缓冲区中的数据复制到共享内存区中对应偏移量的位置
		Sem_post(&ptr->nstored); //挂出一个已经使用槽位信号量
	}
	exit(0);
}
