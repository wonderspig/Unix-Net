typedef struct {
  pthread_t		thread_tid;		/* thread ID 线程ID */
  long			thread_count;	/* # connections handled 这个线程处理的客户数量 */
} Thread;
Thread	*tptr;		/* array of Thread structures; calloc'ed 线程信息结构体指针 */

int				listenfd, nthreads;
socklen_t		addrlen;
pthread_mutex_t	mlock; //互斥锁
