#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int				sockfd;
	//ssize_t			n;
	char			buff[MAXLINE];
	time_t			ticks;
	socklen_t		len;
	struct sockaddr_storage	cliaddr;

	if (argc == 2) //如果参数等于1(只有一个服务名参数)
		sockfd = Udp_server(NULL, argv[1], NULL);
	else if (argc == 3) //如果参数等于2(参数1为主机名,参数2为服务名)
		sockfd = Udp_server(argv[1], argv[2], NULL);
	else //其他的就为参数错误
		sockfd = 0;
		err_quit("usage: daytimeudpsrv [ <host> ] <service or port>");

	for ( ; ; ) { 
		len = sizeof(cliaddr); //计算以下客户端地址结构体的长度
		//从众所周知的地址当中收取数据报,并获得对端的地址信息,填写在cliaddr中
		Recvfrom(sockfd, buff, MAXLINE, 0, (SA *)&cliaddr, &len);
		//打印一下对端的地址信息
		//Sock_ntop转换地址结构中的地址信息和端口号为表达式格式
		printf("datagram from %s\n", Sock_ntop((SA *)&cliaddr, len));

		ticks = time(NULL); //获得当前日历时间
		//把当前日历时间转换成字符串打印到buff中
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); 
		Sendto(sockfd, buff, strlen(buff), 0, (SA *)&cliaddr, len); //发送buff给客户端
	}
}
