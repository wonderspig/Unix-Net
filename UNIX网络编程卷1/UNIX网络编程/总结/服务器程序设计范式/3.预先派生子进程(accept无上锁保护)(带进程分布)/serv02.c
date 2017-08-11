/* include serv02 */
#include	"unp.h"

static int		nchildren;
static pid_t	*pids;

long        *cptr; //指向共享内存的指针
long        *meter(int); //返回的long*指向共享内存

int
main(int argc, char **argv)
{
	int			listenfd, i;
	socklen_t	addrlen;
	void		sig_int(int);
	pid_t		child_make(int, int, int);

	

	if (argc == 3)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		listenfd = 0;
		addrlen = 0;
		err_quit("usage: serv02 [ <host> ] <port#> <#children>");
	nchildren = atoi(argv[argc-1]); //命令行参数的最后一个是预先派生的子进程数,把它转换成数值格式
	cptr = meter(nchildren);
	pids = Calloc(nchildren, sizeof(pid_t)); //动态分配保存子进程的pid数组,大小为子进程数*单个pid的大小

	for (i = 0; i < nchildren; i++) //用循环创建子进程池
		pids[i] = child_make(i, listenfd, addrlen);	/* parent returns */

	Signal(SIGINT, sig_int); //捕捉ctrl+c信号

	for ( ; ; )
		pause();	/* everything done by children 休眠,所有的工作都由子进程去做 */
}
/* end serv02 */

/* include sigint */
void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

		/* 4terminate all children */
	for (i = 0; i < nchildren; i++) //循环杀死每一个子进程
		kill(pids[i], SIGTERM); //向每一个子进程发送SIGTERM信号
	while (wait(NULL) > 0)		/* wait for all children 回收所有的子进程资源*/
		;
	if (errno != ECHILD) //如果错误不是ECHILD(无子进程可等待),则认为是不可接受的,报错！
		err_sys("wait error");

	pr_cpu_time(); //计算一下服务器程序占用的时间

	//打印进程分布
	for(i = 0; i < nchildren; i++){
		printf("进程: %d-------------------%ld", i, *(cptr+i));
	}

	exit(0); //退出服务器程序
}
/* end sigint */
