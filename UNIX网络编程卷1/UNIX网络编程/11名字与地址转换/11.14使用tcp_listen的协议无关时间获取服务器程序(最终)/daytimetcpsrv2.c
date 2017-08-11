#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	socklen_t		len, addrlen;
	char			buff[MAXLINE];
	time_t			ticks;
	struct sockaddr_storage	cliaddr;

	if (argc == 2) //如果参数为1
		//主机名为null,服务名为第一个参数,关心地址结构体大小的调用Tcp_listen
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3) //如果参数为2
		//主机名为参数1,服务名为参数2,关心地址结构体大小的调用Tcp_listen
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: daytimetcpsrv2 [ <host> ] <service or port>"); //否则就出错

	for ( ; ; ) {
		len = sizeof(cliaddr); //计算地址结构体大小
		connfd = Accept(listenfd, (SA *)&cliaddr, &len); //响应客户请求
		//打印客户地址的信息
		printf("connection from %s\n", Sock_ntop((SA *)&cliaddr, len)); //二进制格式地址转换成表达式

		ticks = time(NULL); //获得当前日历时间
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); //日历时间转换成表达式格式后打印到buff
		Write(connfd, buff, strlen(buff)); //输出到连接到客户的套接字

		Close(connfd);
	}
}
