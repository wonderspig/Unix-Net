#include	"unp.h"

int		listenfd, connfd;

void	sig_urg(int);

int
main(int argc, char **argv)
{
	int		n;
	char	buff[100];

	if (argc == 2) //参数不等于1,则出错退出
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(NULL, argv[1], NULL); //
	else if (argc == 3)
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: tcprecv01 [ <host> ] <port#>");

	connfd = Accept(listenfd, NULL, NULL); //响应客户端的请求
	//SIGURG信号为带外数据到达的信号
	Signal(SIGURG, sig_urg); //设置SIGURG信号处理程序
	Fcntl(connfd, F_SETOWN, getpid()); //为套接字设置属主为本进程(带外数据到达的信号会发送给属主进程)

	for ( ; ; ) {
		if ( (n = Read(connfd, buff, sizeof(buff)-1)) == 0) { //从套接字connfd中读取数据,存放在buff中
			printf("received EOF\n");
			exit(0);
		}
		buff[n] = 0;	/* null terminate 缓冲区NULL字符结尾 */
		printf("read %d bytes: %s\n", n, buff); //打印接收到的消息
	}
}

void
sig_urg(int signo)
{
	int		n;
	char	buff[100];

	printf("SIGURG received\n");
	n = Recv(connfd, buff, sizeof(buff)-1, MSG_OOB); //在信号处理程序中读带外数据
	buff[n] = 0;		/* null terminate 缓冲区空字符结尾 */
	printf("read %d OOB byte: %s\n", n, buff); //打印接收到的带外数据
}
