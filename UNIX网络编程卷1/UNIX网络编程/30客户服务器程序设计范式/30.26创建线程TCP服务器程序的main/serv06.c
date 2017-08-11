/* include serv06 */
#include	"unpthread.h"

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	void			sig_int(int);
	void			*doit(void *);
	pthread_t		tid;
	socklen_t		clilen, addrlen;
	struct sockaddr	*cliaddr;

	if (argc == 2)
		//根据命令行参数提供的主机名和端口号,查找IP地址,用查找道德IP地址的地址族和协议创建套接字,把查找到的IP地址绑定到套接字上,监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv06 [ <host> ] <port#>");
	cliaddr = Malloc(addrlen); //动态分配存放客户端地址的地址结构体

	Signal(SIGINT, sig_int); //捕捉ctrl+c信号

	for ( ; ; ) {
		clilen = addrlen;
		connfd = Accept(listenfd, cliaddr, &clilen); //响应客户请求,返回连接到客户端的套接字
		//创建一个线程来处理客户的请求,线程属性为默认,把连接到客户端的套接字当参数传递给线程
		Pthread_create(&tid, NULL, &doit, (void *) connfd); 
	}
}

void *
doit(void *arg)
{
	void	web_child(int);

	Pthread_detach(pthread_self()); //使自己成为分离函数
	web_child((int) arg); //在次函数中响应客户的请求
	Close((int) arg); //关闭连接到客户端的套接字
	return(NULL); //返回null
}
/* end serv06 */

void
sig_int(int signo)
{
	void	pr_cpu_time(void);

	pr_cpu_time(); //计算此服务器的CPU时间
	exit(0);
}
