/* include main */
#include	"unpipc.h"

#define	NBUFF	 10
#define	SEM_MUTEX	"mutex"	 	/* these are args to px_ipc_name() */
#define	SEM_NEMPTY	"nempty"
#define	SEM_NSTORED	"nstored"

int		nitems;					/* read-only by producer and consumer 生产者生产的最多条目数 */
struct {	/* data shared by producer and consumer */
  int	buff[NBUFF]; //按照条目数申请缓冲区
  //三个信号量:
  //mutex为二值信号量(往缓冲区中加入一个条目和往缓冲区中取走一个条目都需要占有此信号量)
  //nempty此信号量记录缓冲区中的空槽位数
  //nstored此信号量记录缓冲区中已使用的槽位数
  sem_t	*mutex, *nempty, *nstored;
} shared;

void	*produce(void *), *consume(void *); //生产者线程函数,消费者线程函数

int
main(int argc, char **argv)
{
	pthread_t	tid_produce, tid_consume;

	if (argc != 2) //命令行参数不等于1
		err_quit("usage: prodcons1 <#items>");
	nitems = atoi(argv[1]); //命令行的第一个参数指明了生产者最多生产多少个未被消费的条目,把它转换成int存放入nitems中

		/* 4create three semaphores 创建三个信号量,其中记录空槽位数的信号量初始值为NBUFF,记录已使用的槽位数的信号量初始值为0*/
	shared.mutex = Sem_open(Px_ipc_name(SEM_MUTEX), O_CREAT | O_EXCL,
							FILE_MODE, 1); 
	shared.nempty = Sem_open(Px_ipc_name(SEM_NEMPTY), O_CREAT | O_EXCL,
							 FILE_MODE, NBUFF);
	shared.nstored = Sem_open(Px_ipc_name(SEM_NSTORED), O_CREAT | O_EXCL,
							  FILE_MODE, 0);

		/* 4create one producer thread and one consumer thread */
	Set_concurrency(2); //最多两个线程并发运行
	Pthread_create(&tid_produce, NULL, produce, NULL); //创建生产者线程,线程函数produce
	Pthread_create(&tid_consume, NULL, consume, NULL); //创建生产者线程,线程函数consume

		/* 4wait for the two threads */
	Pthread_join(tid_produce, NULL); //等待生产者线程结束
	Pthread_join(tid_consume, NULL); //等待消费者线程结束

		/* 4remove the semaphores 删除三个信号量*/
	Sem_unlink(Px_ipc_name(SEM_MUTEX));
	Sem_unlink(Px_ipc_name(SEM_NEMPTY));
	Sem_unlink(Px_ipc_name(SEM_NSTORED));
	exit(0);
}
/* end main */

/* include prodcons */
//生产者线程
void *
produce(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) { //从生产者能生产的最多条目数里循环
		Sem_wait(shared.nempty);	/* wait for at least 1 empty slot 使用一个信号量(维护空槽位的信号量-1)*/
		Sem_wait(shared.mutex); //使用信号量就加锁(二值信号量)
		shared.buff[i % NBUFF] = i;	/* store i into circular buffer 在缓冲区中相印的位置放入相应的值*/
		Sem_post(shared.mutex); //解锁(二值信号量)
		Sem_post(shared.nstored);	/* 1 more stored item 挂出已使用槽位信号量 */
	}
	return(NULL);
}

void *
consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) { //从生产者能生产的最多条目数里循环
		Sem_wait(shared.nstored);		/* wait for at least 1 stored item 使用一个信号量(维护已使用槽位的信号量-1) */
		Sem_wait(shared.mutex); //使用信号量就加锁(二值信号量)
		if (shared.buff[i % NBUFF] != i) //验证缓冲区中对应下标的数值,数值正确就打印
			printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
		Sem_post(shared.mutex); //解锁(二值信号量)
		Sem_post(shared.nempty);		/* 1 more empty slot 挂出空槽位信号量*/
	}
	return(NULL);
}
/* end prodcons */
