typedef struct {
  pthread_t		thread_tid;		/* thread ID */
  long			thread_count;	/* # connections handled */
} Thread;
Thread	*tptr;		/* array of Thread structures; calloc'ed */

#define	MAXNCLI	32 //存放描述符的数组的最大元素个数
//iput是主线程将往该数组中存入的下一个元素的下标
//iget是线程池中某个线程从该数组中取出的下一个元素的下标
//clifd[MAXNCLI]是存放套接字描述符的数组
int					clifd[MAXNCLI], iget, iput;
pthread_mutex_t		clifd_mutex; //互斥锁
pthread_cond_t		clifd_cond; //条件变量
