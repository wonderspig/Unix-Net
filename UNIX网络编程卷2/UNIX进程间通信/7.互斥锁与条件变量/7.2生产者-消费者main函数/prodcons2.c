/* include main */
#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

int		nitems;			/* read-only by producer and consumer */
struct {
  pthread_mutex_t	mutex; //互斥锁
  int	buff[MAXNITEMS];
  int	nput; //下一次存放元素的下标(buff缓冲区中)
  int	nval; //下一次存放的值(buff缓冲区中)
} shared = { PTHREAD_MUTEX_INITIALIZER }; //第一个元素(互斥锁)初始化为PTHREAD_MUTEX_INITIALIZER,其余的初始化为0

void	*produce(void *), *consume(void *);

int
main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3) //参数不等于2,则出错
		err_quit("usage: prodcons2 <#items> <#threads>");
	nitems = min(atoi(argv[1]), MAXNITEMS); //生产者存放的条目数
	nthreads = min(atoi(argv[2]), MAXNTHREADS); //创建生产者线程的数目

	Set_concurrency(nthreads); //并发运行多少线程(貌似linux不需要)
		/* 4start all the producer threads */
	for (i = 0; i < nthreads; i++) { //循环创建线程
		count[i] = 0; //初始化数组元素全部为0
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]); //创建线程,线程函数是produce
	}

		/* 4wait for all the producer threads */
	for (i = 0; i < nthreads; i++) { //等待所有生产者线程结束
		Pthread_join(tid_produce[i], NULL); //Pthread_join函数等待线程结束,回收线程资源
		printf("count[%d] = %d\n", i, count[i]); //打印计数器数组中的各个元素
	}

		/* 4start, then wait for the consumer thread */
	Pthread_create(&tid_consume, NULL, consume, NULL); //创建消费者线程,线程函数是consume
	Pthread_join(tid_consume, NULL); //等待消费者线程结束,回收线程资源

	exit(0);
}
/* end main */

/* include producer */
//生产者线程
void *
produce(void *arg)
{
	for ( ; ; ) {
		Pthread_mutex_lock(&shared.mutex); //加锁互斥量
		if (shared.nput >= nitems) { //下一次存放元素的下标已经超过了生产者存放的条目数
			Pthread_mutex_unlock(&shared.mutex); //解锁互斥量后退出线程
			return(NULL);		/* array is full, we're done */
		}
		shared.buff[shared.nput] = shared.nval; //往缓冲区中放入数据
		shared.nput++; //下一次存放的下标后移
		shared.nval++; //下一次存放的值增1
		Pthread_mutex_unlock(&shared.mutex); //解锁互斥量
		*((int *) arg) += 1; //计数器的值＋1
	}
}
//消费者线程
void *
consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) { //循环buff中的条目数
		if (shared.buff[i] != i) //验证buff数组中的每个元素的值,正确就打印
			printf("buff[%d] = %d\n", i, shared.buff[i]);
	}
	return(NULL);
}
/* end producer */
