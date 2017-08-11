#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

int		nitems;			/* read-only by producer and consumer */
struct {
  pthread_mutex_t	mutex;
  int	buff[MAXNITEMS];
  int	nput;
  int	nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };

void	*produce(void *), *consume(void *);

/* include main */
int
main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3) //如果参数不等于2,则出错
		err_quit("usage: prodcons3 <#items> <#threads>");
	nitems = min(atoi(argv[1]), MAXNITEMS); //生产者存放的条目数
	nthreads = min(atoi(argv[2]), MAXNTHREADS); //代生产的线程总数

		/* 4create all producers and one consumer */
	Set_concurrency(nthreads + 1); //有多少个线程并发运行
	for (i = 0; i < nthreads; i++) { //循环创建生产者线程
		count[i] = 0; //计数器清零
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]); //创建生产者线程,线程函数为produce
	}
	Pthread_create(&tid_consume, NULL, consume, NULL); //创建消费者线程,线程函数为consume

		/* 4wait for all producers and the consumer */
	for (i = 0; i < nthreads; i++) { //循环等待所有生产者线程结束,并回收资源
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]); //打印计数器中的值
	}
	Pthread_join(tid_consume, NULL); //等待消费者线程结束,并回收资源

	exit(0);
}
/* end main */

void *
produce(void *arg)
{
	for ( ; ; ) {
		Pthread_mutex_lock(&shared.mutex);
		if (shared.nput >= nitems) {
			Pthread_mutex_unlock(&shared.mutex);
			return(NULL);		/* array is full, we're done */
		}
		shared.buff[shared.nput] = shared.nval;
		shared.nput++;
		shared.nval++;
		Pthread_mutex_unlock(&shared.mutex);
		*((int *) arg) += 1;
	}
}

/* include consume */
void
consume_wait(int i)
{
	for ( ; ; ) {
		Pthread_mutex_lock(&shared.mutex);
		if (i < shared.nput) { //判断生产者线程是否已经生产到了第i个条目
			Pthread_mutex_unlock(&shared.mutex);
			return;			/* an item is ready */
		}
		Pthread_mutex_unlock(&shared.mutex);
	}
}

void *
consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) {
		consume_wait(i); //此函数中等待生产者线程产生到了第i个条目,才会返回
		if (shared.buff[i] != i) //验证缓冲区中数据的正确性
			printf("buff[%d] = %d\n", i, shared.buff[i]); //打印缓冲区中的数据
	}
	return(NULL);
}
/* end consume */
