/* include serv08 */
#include	"unpthread.h"
#include	"pthread08.h"

static int			nthreads; //线程的总个数
pthread_mutex_t		clifd_mutex = PTHREAD_MUTEX_INITIALIZER; //初始化互斥锁
pthread_cond_t		clifd_cond = PTHREAD_COND_INITIALIZER; //初始化条件变量

int
main(int argc, char **argv)
{
	int			i, listenfd, connfd;
	void		sig_int(int), thread_make(int);
	socklen_t	addrlen, clilen;
	struct sockaddr	*cliaddr;

	if (argc == 3)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv08 [ <host> ] <port#> <#threads>");
	cliaddr = Malloc(addrlen); //动态分配存放客户端地址的地址结构体空间

	nthreads = atoi(argv[argc-1]); //命令行参数的最后一个参数是需要预先分配的线程个数
	tptr = Calloc(nthreads, sizeof(Thread)); //动态生成线程信息结构体的数组
	//iput是主线程将往该数组中存入的下一个元素的下标初始化为0
	//iget是线程池中某个线程从该数组中取出的下一个元素的下标初始化为0
	iget = iput = 0;

		/* 4create all the threads */
	for (i = 0; i < nthreads; i++) //在thread_make函数中循环创建线程并做相应处理
		thread_make(i);		/* only main thread returns */

	Signal(SIGINT, sig_int); //捕捉ctrl+c信号

	for ( ; ; ) {
		clilen = addrlen;
		connfd = Accept(listenfd, cliaddr, &clilen); //响应客户请求,并返回连接到客户的套接字

		Pthread_mutex_lock(&clifd_mutex); //给全局变量(线程信息结构体的数组)加锁
		clifd[iput] = connfd; //把描述符存入
		if (++iput == MAXNCLI) //如果下标已经到达最大值,则回绕
			iput = 0;

		//如下条件成立说明主线程已经往数组中放入了过多的描述符,而仍没有线程去取出(一下子有太多的客户连接,线程忙不过来)
		if (iput == iget) //如果线程取出描述符的下标和主线程放入描述符的下标相等,则出错
			err_quit("iput = iget = %d", iput);
		Pthread_cond_signal(&clifd_cond); //给条件变量发消息
		Pthread_mutex_unlock(&clifd_mutex); //解锁互斥锁
	}
}
/* end serv08 */

void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

	pr_cpu_time(); //计算cpu资源的使用情况

	for (i = 0; i < nthreads; i++) //打印各个线程各自处理了多少个客户请求
		printf("thread %d, %ld connections\n", i, tptr[i].thread_count);

	exit(0);
}
