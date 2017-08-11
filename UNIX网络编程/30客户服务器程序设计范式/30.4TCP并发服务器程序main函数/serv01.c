/* include serv01 */
#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	void				sig_chld(int), sig_int(int), web_child(int);
	socklen_t			clilen, addrlen;
	struct sockaddr		*cliaddr;

	if (argc == 2)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv01 [ <host> ] <port#>");
	cliaddr = Malloc(addrlen); //动态分配保存客户地址的地址结构体空间

	Signal(SIGCHLD, sig_chld); //注册子进程结束信号SIGCHLD的信号处理程序
	Signal(SIGINT, sig_int); //注册ctrl+c的信号处理函数

	for ( ; ; ) {
		clilen = addrlen; //把客户地址结构体大小复制一份
		if ( (connfd = accept(listenfd, cliaddr, &clilen)) < 0) { //响应客户端请求,返回连接到客户端的套接字
			if (errno == EINTR) //被信号打断则继续
				continue;		/* back to for() */
			else
				err_sys("accept error"); //其他错误无法忍受
		}
		//创建一个子进程
		if ( (childpid = Fork()) == 0) {	/* child process 在子进程中 */
			Close(listenfd);	/* close listening socket 关闭监听套接字 */
			web_child(connfd);	/* process request 在这个函数中响应客户的请求*/
			exit(0); //退出子进程
		}
		Close(connfd);			/* parent closes connected socket 父进程中关闭连接到客户端的套接字,继续监听等待客户端的连接 */
	}
}
/* end serv01 */

/* include sigint */
void
sig_int(int signo)
{
	void	pr_cpu_time(void); 

	pr_cpu_time(); //打印cpu时间
	exit(0);
}
/* end sigint */
