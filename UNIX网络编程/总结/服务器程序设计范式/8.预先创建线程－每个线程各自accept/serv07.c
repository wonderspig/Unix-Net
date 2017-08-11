/* include serv07 */
#include	"unpthread.h"
#include	"pthread07.h"

pthread_mutex_t	mlock = PTHREAD_MUTEX_INITIALIZER; //初始化互斥锁

int
main(int argc, char **argv)
{
	int		i;
	void	sig_int(int), thread_make(int);

	if (argc == 3)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		listenfd = 0;
		addrlen = 0;
		err_quit("usage: serv07 [ <host> ] <port#> <#threads>");
	nthreads = atoi(argv[argc-1]); //命令行参数的最后一个参数是预先分配的线程个数
	tptr = Calloc(nthreads, sizeof(Thread)); //动态创建线程信息结构体数组

	for (i = 0; i < nthreads; i++) //创建规定数目的线程并做相应的操作
		thread_make(i);			/* only main thread returns */

	Signal(SIGINT, sig_int); //捕捉ctrl+c信号

	for ( ; ; )
		pause();	/* everything done by threads 所有的操作都在线程中完成 */
}
/* end serv07 */

void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

	pr_cpu_time(); //计算该服务器的时间

	for (i = 0; i < nthreads; i++) //打印每个线程处理的客户数目
		printf("thread %d, %ld connections\n", i, tptr[i].thread_count);

	exit(0); //退出此服务器程序
}
