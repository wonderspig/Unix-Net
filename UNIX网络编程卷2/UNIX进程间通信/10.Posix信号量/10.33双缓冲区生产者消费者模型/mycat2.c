/* include main */
#include	"unpipc.h"

#define	NBUFF	 8

struct {	/* data shared by producer and consumer */
  struct {
    char	data[BUFFSIZE];			/* a buffer 缓冲区*/
    ssize_t	n;						/* count of #bytes in the buffer 缓冲区中当前的字节数 */
  } buff[NBUFF];					/* NBUFF of these buffers/counts 缓冲区结构体数组 */
  //三个信号量,二值信号量,空槽位信号量,已使用槽位信号量
  sem_t	mutex, nempty, nstored;		/* semaphores, not pointers */
} shared;

int		fd;							/* input file to copy to stdout 输入文件并拷贝到标准输出*/
void	*produce(void *), *consume(void *); //生产者线程,消费者线程

int
main(int argc, char **argv)
{
	pthread_t	tid_produce, tid_consume;

	if (argc != 2) //如果参数不等于1,则出错
		err_quit("usage: mycat2 <pathname>");

	fd = Open(argv[1], O_RDONLY); //只读打开命令行第一个参数所代表的文件

		/* 4initialize three semaphores 初始化三个信号量*/
	Sem_init(&shared.mutex, 0, 1);
	Sem_init(&shared.nempty, 0, NBUFF);
	Sem_init(&shared.nstored, 0, 0);

		/* 4one producer thread, one consumer thread */
	Set_concurrency(2); //最大并发运行的线程为2
	Pthread_create(&tid_produce, NULL, produce, NULL);	/* reader thread 创建生产者线程,读文件*/
	Pthread_create(&tid_consume, NULL, consume, NULL);	/* writer thread 创建消费者线程,写文件*/
	//等待两个线程结束
	Pthread_join(tid_produce, NULL); 
	Pthread_join(tid_consume, NULL);
	//销毁三个信号量
	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.nempty);
	Sem_destroy(&shared.nstored);
	exit(0);
}
/* end main */

/* include prodcons */
//生产者线程
void *
produce(void *arg)
{
	int		i;

	for (i = 0; ; ) {
		Sem_wait(&shared.nempty);	/* wait for at least 1 empty slot 获取空槽位信号量*/

		Sem_wait(&shared.mutex); //加锁(二值信号量)
			/* 4critical region 如果数据缓冲区是在链表中维护的,这里是为了防止干扰,操作链表,移出链表中的数据 */
		Sem_post(&shared.mutex); //解锁(二值信号量)
		//读文件,存放入shared.buff[i].data中,读了多少字节,存放入shared.buff[i].n中
		shared.buff[i].n = Read(fd, shared.buff[i].data, BUFFSIZE); 
		if (shared.buff[i].n == 0) { //如果读到文件尾端
			Sem_post(&shared.nstored);	/* 1 more stored item 挂出一个已使用槽位信号量*/
			return(NULL); //函数返回
		}
		if (++i >= NBUFF) //如果下标大于了数组下标,则下标恢复成0(环形缓冲区)
			i = 0;					/* circular buffer */

		Sem_post(&shared.nstored);	/* 1 more stored item 挂出一个已使用槽位信号量 */
	}
}
//消费者线程
void *
consume(void *arg)
{
	int		i;

	for (i = 0; ; ) {
		Sem_wait(&shared.nstored);		/* wait for at least 1 stored item 获得一个已使用槽位信号量*/

		Sem_wait(&shared.mutex); //加锁(二值信号量)
			/* 4critical region 如果数据缓冲区是在链表中维护的,这里是为了防止干扰,操作链表,移出链表中的数据 */
		Sem_post(&shared.mutex); //解锁(二值信号量)

		if (shared.buff[i].n == 0) //缓冲区中已经没有数据了
			return(NULL); //函数返回
		//把shared.buff[i].data中的数据,写到标准输出,写shared.buff[i].n个字节
		Write(STDOUT_FILENO, shared.buff[i].data, shared.buff[i].n); 
		if (++i >= NBUFF) //如果下标大于了数组下标,则下标恢复成0(环形缓冲区)
			i = 0;					/* circular buffer */

		Sem_post(&shared.nempty);		/* 1 more empty slot 挂出一个空槽位信号量*/
	}
}
/* end prodcons */
