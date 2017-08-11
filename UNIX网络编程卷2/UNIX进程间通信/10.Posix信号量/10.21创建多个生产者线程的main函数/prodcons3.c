/* include main */
#include	"unpipc.h"

#define	NBUFF	 	 10 //缓冲区大小
#define	MAXNTHREADS	100 //最大的线程数量

int		nitems, nproducers;		/* read-only by producer and consumer */

struct {	/* data shared by producers and consumer */
  int	buff[NBUFF]; //存放数据的缓冲区
  int	nput; //缓冲区中下一个要存入数据的下标
  int	nputval; //缓冲区中下一个要存入数据的值
  //三个信号量,一个二值信号量,一个空槽位信号量,一个已使用槽位信号量
  sem_t	mutex, nempty, nstored;		/* semaphores, not pointers */
} shared;

void	*produce(void *), *consume(void *);

int
main(int argc, char **argv)
{
	int		i, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3) //如果参数不等于2,则出错
		err_quit("usage: prodcons3 <#items> <#producers>");
	nitems = atoi(argv[1]); //命令行第一个参数为条目数
	nproducers = min(atoi(argv[2]), MAXNTHREADS); //命令行第二个参数为生产者生产线程的数量

		/* 4initialize three semaphores 初始化三个信号量 */
	Sem_init(&shared.mutex, 0, 1);
	Sem_init(&shared.nempty, 0, NBUFF);
	Sem_init(&shared.nstored, 0, 0);

		/* 4create all producers and one consumer */
	Set_concurrency(nproducers + 1); //最大允许的线程数为生产者线程数+一个消费者线程数
	for (i = 0; i < nproducers; i++) { //循环创建生产者线程
		count[i] = 0; //计数器清0
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]); //创建生产者线程,线程函数为produce
	}
	Pthread_create(&tid_consume, NULL, consume, NULL); //创建消费者线程,线程函数为consume

		/* 4wait for all producers and the consumer */
	for (i = 0; i < nproducers; i++) { //循环等待所有生产者线程结束
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	//打印计数器中的值
	}
	Pthread_join(tid_consume, NULL); //等待消费者线程结束
	//销毁三个信号量
	Sem_destroy(&shared.mutex);
	Sem_destroy(&shared.nempty);
	Sem_destroy(&shared.nstored);
	exit(0);
}
/* end main */

/* include produce */
//生产者线程
void *
produce(void *arg)
{
	for ( ; ; ) {
		Sem_wait(&shared.nempty);	/* wait for at least 1 empty slot 使用一个空槽位信号量 */
		Sem_wait(&shared.mutex); //加锁(二值信号量)

		if (shared.nput >= nitems) { //如果下一次压入缓冲区的下标大于生产者线程应该生产的最大条目数(该生产者已经生产完了约定的所有条目)
			//准备退出
			Sem_post(&shared.nempty); //释放一个空槽位信号量(前面加了一个,但并没有往缓冲区存放数据)
			Sem_post(&shared.mutex); //解锁(二值信号量)
			return(NULL);			/* all done */
		}
		//该线程还有没有生产完的条目
		shared.buff[shared.nput % NBUFF] = shared.nputval; //往约定的下标处存放数据
		shared.nput++; //下次存放缓冲区的下标后移一格
		shared.nputval++; //下一次存放的数据,自增1

		Sem_post(&shared.mutex); //解锁(二值信号量)
		Sem_post(&shared.nstored);	/* 1 more stored item 释放一个已使用槽位信号量*/
		*((int *) arg) += 1; //计数器相应的值+1
	}
}
/* end produce */

/* include consume */
//消费者线程
void *
consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) { //循环遍历所有条目
		Sem_wait(&shared.nstored);		/* wait for at least 1 stored item 获取一个已使用槽位数的信号量*/
		Sem_wait(&shared.mutex); //加锁(二值信号量)

		if (shared.buff[i % NBUFF] != i) //验证缓冲区中数值的准确性,数据无误则打印
			printf("error: buff[%d] = %d\n", i, shared.buff[i % NBUFF]);

		Sem_post(&shared.mutex); //解锁(二值信号量)
		Sem_post(&shared.nempty);		/* 1 more empty slot 释放一个空槽位信号量*/
	}
	return(NULL);
}
/* end consume */
