#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	socklen_t		len;
	char			buff[MAXLINE];
	time_t			ticks;
	struct sockaddr_storage	cliaddr;

	if (argc != 2) //如果参数不为1,则出错退出
		err_quit("usage: daytimetcpsrv1 <service or port#>");

	//以主机名空,服务名为参数1,不关心listen的地址大小来调用Tcp_listen
	listenfd = Tcp_listen(NULL, argv[1], NULL); 

	for ( ; ; ) {
		len = sizeof(cliaddr); //计算客户地址结构体大小
		connfd = Accept(listenfd, (SA *)&cliaddr, &len); //响应客户请求
		//打印客户端地址信息
		printf("connection from %s\n", Sock_ntop((SA *)&cliaddr, len)); //把客户端地址转换成表达式

		ticks = time(NULL); //获得当前日历时间
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); //把日历时间转换成表达式打印到buff中
		Write(connfd, buff, strlen(buff)); //写到连接到客户端的套接字connfd

		Close(connfd); //关闭客户端套接字
	}
}
