/* include globals */
#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

		/* globals shared by threads */
int		nitems;				/* read-only by producer and consumer 生产者线程所写入的条目数*/
int		buff[MAXNITEMS]; //缓冲区(MAXNITEMS为最大条目数)
struct {
  pthread_mutex_t	mutex;  //互斥锁
  int				nput;	/* next index to store 下一个要写入数据的下标*/
  int				nval;	/* next value to store  下一个要写入数据的值*/
} put = { PTHREAD_MUTEX_INITIALIZER }; //初始化互斥量为PTHREAD_MUTEX_INITIALIZER,其余成员初始化为0

struct {
  pthread_mutex_t	mutex; //互斥锁
  pthread_cond_t	cond; //条件变量
  int				nready;	/* number ready for consumer 生产者每生产一个数据,会把这个值+1 */
} nready = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };
/* end globals */

void	*produce(void *), *consume(void *);

/* include main main函数没有变动 */
int
main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3)
		err_quit("usage: prodcons6 <#items> <#threads>");
	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]), MAXNTHREADS);

	Set_concurrency(nthreads + 1);
		/* 4create all producers and one consumer */
	for (i = 0; i < nthreads; i++) {
		count[i] = 0;
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}
	Pthread_create(&tid_consume, NULL, consume, NULL);

		/* wait for all producers and the consumer */
	for (i = 0; i < nthreads; i++) {
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	
	}
	Pthread_join(tid_consume, NULL);

	exit(0);
}
/* end main */

/* include prodcons 生产者线程 */
void *
produce(void *arg)
{
	for ( ; ; ) {
		Pthread_mutex_lock(&put.mutex); //上锁互斥量
		if (put.nput >= nitems) { //如果下标大于最大的条目数(已经写完了所有的条目)
			Pthread_mutex_unlock(&put.mutex); //解锁互斥量,退出线程
			return(NULL);		/* array is full, we're done */
		}
		buff[put.nput] = put.nval; //把值写入缓存
		put.nput++; //下一次写入的下标向后移动一位
		put.nval++; //写入的值自增1
		Pthread_mutex_unlock(&put.mutex); //解锁互斥量

		Pthread_mutex_lock(&nready.mutex); //加锁等待条件变量
		if (nready.nready == 0) //如果生产线程生产的数据已经给消费完了,那么要向条件变量发个信号
			Pthread_cond_signal(&nready.cond); //唤醒阻塞在条件变量中的线程
		nready.nready++; //每生产一个数据,就把这个值+1
		Pthread_mutex_unlock(&nready.mutex); //解锁互斥量

		*((int *) arg) += 1; //计数器中,相对于本进程的值自增1
	}
}
//消费者线程
void *
consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) { //循环遍历缓冲区中所有条目
		Pthread_mutex_lock(&nready.mutex); //加速互斥量
		while (nready.nready == 0) //如果已经没有数据可供消费
			Pthread_cond_wait(&nready.cond, &nready.mutex); //睡眠等待条件变量
		nready.nready--; //每消费一个数据,就把这个值-1
		Pthread_mutex_unlock(&nready.mutex);

		if (buff[i] != i) //验证缓冲区中的数值是否正确
			printf("buff[%d] = %d\n", i, buff[i]); //打印这个不正确的数值
	}
	return(NULL);
}
/* end prodcons */
