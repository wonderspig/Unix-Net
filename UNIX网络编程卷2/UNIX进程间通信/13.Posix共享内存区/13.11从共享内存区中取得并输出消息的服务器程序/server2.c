#include	"cliserv2.h"

int
main(int argc, char **argv)
{
	int		fd, index, lastnoverflow, temp;
	long	offset;
	struct shmstruct	*ptr;

	if (argc != 2) //如果命令行参数不等于2,则出错
		err_quit("usage: server2 <name>");

		/* 4create shm, set its size, map it, close descriptor */
	shm_unlink(Px_ipc_name(argv[1]));		/* OK if this fails 删除共享内存区(防止共享内存区已经存在) */
	fd = Shm_open(Px_ipc_name(argv[1]), O_RDWR | O_CREAT | O_EXCL, FILE_MODE); //打开共享内存区对象
	ptr = Mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射共享内存区对象
	Ftruncate(fd, sizeof(struct shmstruct)); //调整共享内存区对象大小
	Close(fd); //关闭共享内存区对象,不影响已经映射的内存

		/* 4initialize the array of offsets 初始化每条消息的偏移量数组*/
	//第一条消息偏移量为0,第二条消息偏移量为MESGSIZE,第三条消息偏移量为,2*MESGSIZE,宏MESGSIZE为每条消息的长度
	for (index = 0; index < NMESG; index++) 
		ptr->msgoff[index] = index * MESGSIZE;

		/* 4initialize the semaphores in shared memory 初始化四个信号量,全部都有进程间共享属性,第三个参数为信号量的初始值*/
	Sem_init(&ptr->mutex, 1, 1);
	Sem_init(&ptr->nempty, 1, NMESG);
	Sem_init(&ptr->nstored, 1, 0);
	Sem_init(&ptr->noverflowmutex, 1, 1);

		/* 4this program is the consumer */
	index = 0; //初始化偏移量(索引)
	lastnoverflow = 0; //初始化溢出计数器
	for ( ; ; ) {
		Sem_wait(&ptr->nstored); //获得一个已使用槽位信号量
		Sem_wait(&ptr->mutex); //加锁(二值信号量)
		offset = ptr->msgoff[index]; //第一条消息的位置从0开始(取出刚才初始化好的数组中的每一个成员都是一条消息的偏移量)
		printf("index = %d: %s\n", index, &ptr->msgdata[offset]); //打印消息的索引,消息的内容
		if (++index >= NMESG) //索引自增1,超过数组下标则归0(环形缓冲区)
			index = 0;				/* circular buffer */
		Sem_post(&ptr->mutex); //解锁(二值信号量)
		Sem_post(&ptr->nempty); //挂出一个空槽位信号量

		Sem_wait(&ptr->noverflowmutex); //获得一个溢出计数器的二值信号量
		temp = ptr->noverflow;		/* don't printf while mutex held 取出溢出计数器的值 */
		Sem_post(&ptr->noverflowmutex); //解锁溢出计数器的二值信号量
		if (temp != lastnoverflow) { //把当前溢出计数器的值和上一次循环溢出计数器的值进行表,如果不同就代表有数据溢出(缓冲区中没有空槽位了)
			printf("noverflow = %d\n", temp); //打印消息
			lastnoverflow = temp; //更新当前溢出计数器的值
		}
	}

	exit(0);
}
