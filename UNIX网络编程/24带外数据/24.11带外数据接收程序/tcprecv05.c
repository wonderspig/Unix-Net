#include	"unp.h"

int		listenfd, connfd;

void	sig_urg(int);

int
main(int argc, char **argv)
{
	int		size;

	if (argc == 2) //一个命令行参数
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(NULL, argv[1], NULL);
	else if (argc == 3) //两个命令行参数
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: tcprecv05 [ <host> ] <port#>");

	size = 4096;
	Setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)); //设置套接字选项,定义发送缓冲区为4096

	connfd = Accept(listenfd, NULL, NULL); //响应客户端请求
	//SIGURG信号标识有带外数据到来
	Signal(SIGURG, sig_urg); //设置SIGURG信号处理程序
	Fcntl(connfd, F_SETOWN, getpid()); //设置套接字属主进程(信号会发送给属主进程)

	for ( ; ; )
		pause(); //死循环等待信号到来
}

void
sig_urg(int signo)
{
	int		n;
	char	buff[2048];

	printf("SIGURG received\n");
	n = Recv(connfd, buff, sizeof(buff)-1, MSG_OOB); //接收带外数据
	buff[n] = 0;		/* null terminate */
	printf("read %d OOB byte\n", n);
}
