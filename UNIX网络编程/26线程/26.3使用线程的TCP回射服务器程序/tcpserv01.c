#include	"unpthread.h"

static void	*doit(void *);		/* each thread executes this function */

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	pthread_t		tid;
	socklen_t		addrlen, len;
	struct sockaddr	*cliaddr;

	if (argc == 2) //命令行参数等于2
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3) //命令行参数等于3
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: tcpserv01 [ <host> ] <service or port>");

	cliaddr = Malloc(addrlen); //动态分配存放客户地址的空间

	for ( ; ; ) {
		len = addrlen; //存放客户地址结构体的大小
		connfd = Accept(listenfd, cliaddr, &len); //响应客户请求
		Pthread_create(&tid, NULL, &doit, (void *) connfd); //创建线程,属性默认,把连接到客户的套接字传递给线程
	}
}

static void *
doit(void *arg)
{
	Pthread_detach(pthread_self()); //是自己成为分离线程
	str_echo((int) arg);	/* same function as before 在此函数中提供对客户的服务 */
	Close((int) arg);		/* done with connected socket 线程结束不会关闭套接字,所以需要手动关闭*/
	return(NULL);
}
