/* include serv05a */
#include	"unp.h"
#include	"child.h"

static int		nchildren;

int
main(int argc, char **argv)
{
	int			listenfd, i, navail, maxfd, nsel, connfd, rc;
	void		sig_int(int);
	pid_t		child_make(int, int, int);
	ssize_t		n;
	fd_set		rset, masterset;
	socklen_t	addrlen, clilen;
	struct sockaddr	*cliaddr;

	if (argc == 3)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv05 [ <host> ] <port#> <#children>");

	FD_ZERO(&masterset); //初始化清空主描述符集
	FD_SET(listenfd, &masterset); //把监听套接字加入主描述符集
	maxfd = listenfd; //暂时就只有一个监听套接字,所以监听套接字就是最大描述符
	cliaddr = Malloc(addrlen); //动态分配存储客户端地址的地址结构体空间 

	nchildren = atoi(argv[argc-1]); //命令行最后一个参数是预先分配的子进程个数
	navail = nchildren; //预先分配的子进程个数复制一份(用作记录当前空闲的子进程数)
	cptr = Calloc(nchildren, sizeof(Child)); //分配用来存放"子进程信息"的"子进程信息结构体数组"的空间

		/* 4prefork all the children */
	for (i = 0; i < nchildren; i++) { //用循环创建命令行参数规定的子进程数目
		//child_make函数中:创建命令行规定数目的子进程,记录并填写子进程的信息到子进程结构体中
		//父进程在这个子进程信息结构体数组中维护连接到子进程的全双工管道和子进程是否空闲的状态
		//在子进程中把连接到父进程中的全双工管道复制到标准错误,且关闭不需要的套接字。
		//在子进程中,最后读取父进程传递过来的连接到客户端的套接字,并响应客户请求
		child_make(i, listenfd, addrlen);	/* parent returns 这个函数只返回父进程*/
		FD_SET(cptr[i].child_pipefd, &masterset); //在父进程中,把连接到子进程的全双工管道设置到主描述符集中
		maxfd = max(maxfd, cptr[i].child_pipefd); //已经有除了监听套接字意外的其他套接字了,需要计算出最大值来调用select
	}

	Signal(SIGINT, sig_int); //捕捉ctrl+c信号

	for ( ; ; ) {
		rset = masterset; //因为select会更改描述符集,所以复制一份,准备调用select
		if (navail <= 0) //检查当前的空闲子进程数目小于等于0
			/* turn off if no available children 则清除描述符集中的监听描述符,没有子进程空闲,所以不监听了 */
			FD_CLR(listenfd, &rset);	
		nsel = Select(maxfd + 1, &rset, NULL, NULL, NULL); //调用select关心描述符的读状态

			/* 4check for new connections */
		if (FD_ISSET(listenfd, &rset)) { //如果是监听描述符的读状态准备好了
			clilen = addrlen; //复制一份客户端地址结构体的大小
			connfd = Accept(listenfd, cliaddr, &clilen); //响应客户端请求

			//查找哪个子进程数组空闲状态,在跳出循环的时候,i下标所指明的子进程结构体信息所代表的子进程,正处于空闲状态
			for (i = 0; i < nchildren; i++) 
				if (cptr[i].child_status == 0)
					break;				/* available 已经找到*/

			if (i == nchildren) //说明没找到(没有子进程处于空闲状态)
				err_quit("no available children");
			cptr[i].child_status = 1;	/* mark child as busy 把找到的这个子进程标志为忙碌状态 */
			cptr[i].child_count++; //这个子进程处理的客户数量递增1
			navail--; //空闲子进程数目-1

			n = Write_fd(cptr[i].child_pipefd, "", 1, connfd); //把描述符传递给这个已经找到的空闲的子进程
			Close(connfd); //关闭连接到这个客户端的描述符(传递描述符实际上是复制一份再传递的)
			if (--nsel == 0) //条件满足表示已经处理完了所有select返回的已经准备好的描述符
				continue;	/* all done with select() results */
		}

			/* 4find any newly-available children */
		for (i = 0; i < nchildren; i++) { //用循环操作子进程信息结构体数组，看看是哪个一子进程已经处理好，返回空闲状态了
			if (FD_ISSET(cptr[i].child_pipefd, &rset)) { //如果有哪一个子进程准备好了
				//从连接到这个子进程的全双工管道中,读取数据(不关心这个数据是什么，总之就是这个子进程又处于空闲状态了)
				if ( (n = Read(cptr[i].child_pipefd, &rc, 1)) == 0) 
					err_quit("child %d terminated unexpectedly", i);
				cptr[i].child_status = 0; //操作子进程信息结构体数组,把这个子进程的状态修改为空闲
				navail++; //空闲子进程数目+1
				if (--nsel == 0) //条件满足表示已经处理完了所有select返回的已经准备好的描述符
					break;	/* all done with select() results */
			}
		}
	}
}
/* end serv05a */
//ctrl+c的处理函数
void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

		/* 4terminate all children */
	for (i = 0; i < nchildren; i++) //向每一个子进程发送SIGTERM,从而杀死它们
		kill(cptr[i].child_pid, SIGTERM);
	while (wait(NULL) > 0)		/* wait for all children 等待所有子进程结束 */
		;
	if (errno != ECHILD) //如果错误不是"没有子进程可供等待",则报错退出
		err_sys("wait error");

	pr_cpu_time(); //计算服务器的的资源使用

	for (i = 0; i < nchildren; i++) //打印每一个子进程处理了多少个客户端
		printf("child %d, %ld connections\n", i, cptr[i].child_count);

	exit(0);
}
