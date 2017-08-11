/* include globals */
#include	"unpipc.h"

#define	NBUFF	 	 10 //缓冲区大小
#define	MAXNTHREADS	100 //生产者线程数最大值

int		nitems, nproducers, nconsumers;		/* read-only 者三个值由命令行传递,线程中只读*/

struct {	/* data shared by producers and consumers */
  int	buff[NBUFF]; //存放数据的缓冲区
  int	nput;			/* item number: 0, 1, 2, ... 下一次存入缓冲区的下标*/
  int	nputval;		/* value to store in buff[] 下一次存入缓冲区的值*/
  int	nget;			/* item number: 0, 1, 2, ... 下一次从缓冲区取出的下标*/
  int	ngetval;		/* value fetched from buff[] 下一次从缓冲区取出的值*/
  //三个信号量,二值信号量,空槽位信号量,已使用槽位信号量
  sem_t	mutex, nempty, nstored;		/* semaphores, not pointers */
} shared;

void	*produce(void *), *consume(void *); //生产者线程,消费者线程
/* end globals */

/* include main */
int
main(int argc, char **argv)
{
	int		i, prodcount[MAXNTHREADS], conscount[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume[MAXNTHREADS];

	if (argc != 4) //如果命令行参数不等于3,则出错
		err_quit("usage: prodcons4 <#items> <#producers> <#consumers>");
	nitems = atoi(argv[1]); //生产者生产的条目总数
	nproducers = min(atoi(argv[2]), MAXNTHREADS); //生产者线程数目
	nconsumers = min(atoi(argv[3]), MAXNTHREADS); //消费者线程数目

		/* 4initialize three semaphores 初始化三个信号量 */
	Sem_init(&shared.mutex, 0, 1);
	Sem_init(&shared.nempty, 0, NBUFF);
	Sem_init(&shared.nstored, 0, 0);

		/* 4create all producers and all consumers */
	Set_concurrency(nproducers + nconsumers); //最大并发的线程数,生产者线程数+消费者线程数
	for (i = 0; i < nproducers; i++) { //循环创建规定数量的生产者线程
		prodcount[i] = 0; //生产者线程计数器清零
		Pthread_create(&tid_produce[i], NULL, produce, &prodcount[i]); //创建生产者线程,线程函数为produce
	}
	for (i = 0; i < nconsumers; i++) { //循环创建规定数量的消费者线程
		conscount[i] = 0; //消费者线程计数器清零
		Pthread_create(&tid_consume[i], NULL, consume, &conscount[i]); //创建消费者线程,线程函数为consume
	}

		/* 4wait for all producers and all consumers */
	for (i = 0; i < nproducers; i++) { //等待生产者线程结束
		Pthread_join(tid_produce[i], NULL);
		printf("producer count[%d] = %d\n", i, prodcount[i]);	
	}
	for (i = 0; i < nconsumers; i++) { //等待消费者线程结束
		Pthread_join(tid_consume[i], NULL);
		printf("consumer count[%d] = %d\n", i, conscount[i]);	
	}
	//销毁三个信号量
	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.nempty);
	Sem_destroy(&shared.nstored);
	exit(0);
}
/* end main */

/* include produce */
//生产者线程函数
void *
produce(void *arg)
{
	for ( ; ; ) {
		Sem_wait(&shared.nempty);	/* wait for at least 1 empty slot 获得一个空槽位信号量 */
		Sem_wait(&shared.mutex); //加锁(二值信号量)

		if (shared.nput >= nitems) { //如果下一次压入缓冲区的下标大于生产者线程应该生产的最大条目数(该生产者已经生产完了约定的所有条目)
			Sem_post(&shared.nstored);	/* let consumers terminate 增加一个已使用槽位信号量:
											用来告诉消费者线程,生产者线程已经生产完了所有的数据*/
			Sem_post(&shared.nempty); //释放一个空槽位信号量(前面加了一个,但并没有往缓冲区存放数据)
			Sem_post(&shared.mutex); //解锁(二值信号量)
			return(NULL);			/* all done */
		}
		//生产者还有数据需要生产
		shared.buff[shared.nput % NBUFF] = shared.nputval; //向缓冲区中存入相应的值
		shared.nput++; //下一次存入缓冲区的下标后移一格
		shared.nputval++; //下一次存入缓冲区的数据+1

		Sem_post(&shared.mutex); //解锁(二值信号量)
		Sem_post(&shared.nstored);	/* 1 more stored item 释放已使用槽位信号量*/
		*((int *) arg) += 1; //生产者线程计数器本线程位置的值+1
	}
}
/* end produce */

/* include consume */
//消费者线程
void *
consume(void *arg)
{
	int		i;

	for ( ; ; ) {
		//已使用的槽位信号量初始化值为0,所以当消费完所有的数据后,循环到达这里会阻塞
		//所以生产者线程应道在退出之前,把这个信号量+1,从而告诉消费者线程我已经生产完了全部的数据,消费者线程会在紧接着的if语句中确定这一点,从而退出
		Sem_wait(&shared.nstored);	/* wait for at least 1 stored item 获得一个已使用槽位信号量*/
		Sem_wait(&shared.mutex); //加锁(二值信号量)

		if (shared.nget >= nitems) { //如果下一次从缓冲区中取出的下标等于最大的条目数(说明数据应该已经消费完了)
			Sem_post(&shared.nstored); //释放已使用槽位数信号量(前面使用了这个信号量,但是并没有消费数据)
			Sem_post(&shared.mutex); //解锁(二值信号量)
			return(NULL);			/* all done */
		}

		i = shared.nget % NBUFF; //访问缓冲区的数组下标
		if (shared.buff[i] != shared.ngetval) //验证缓冲区中数据的正确性,正确就打印
			printf("error: buff[%d] = %d\n", i, shared.buff[i]);
		shared.nget++; //下一次从缓冲区中获取数据的下标向后移动一格
		shared.ngetval++; //下一次存放入缓冲区的数据自增1

		Sem_post(&shared.mutex); //解锁(二值信号量)
		Sem_post(&shared.nempty);	/* 1 more empty slot 释放一个空槽位信号量*/
		*((int *) arg) += 1; //消费者线程计数器本线程位置的值+1
	}
}
/* end consume */
