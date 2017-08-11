#include	"unp.h"

int
main(int argc, char **argv)
{
	int				sockfd, n;
	char			recvline[MAXLINE + 1];
	socklen_t		salen;
	struct sockaddr	*sa;

	if (argc != 3) //如果参数个数不是2个,则报错退出
		err_quit("usage: daytimeudpcli1 <hostname/IPaddress> <service/port#>");

	//调用Udp_client,参数1为主机名,参数2为服务名,sa中存放未连接的UDP套接字地址,salen中存放地址长度
	sockfd = Udp_client(argv[1], argv[2], (SA **) &sa, &salen);

	//打印信息,UDP套接字将发送到哪个地址
	//Sock_ntop_host返回并转换地址当中的主机名部分到表达式格式
	printf("sending to %s\n", Sock_ntop_host(sa, salen)); 

	//利用Udp_client返回的套接字,和地址,发送一个字节数据
	Sendto(sockfd, "", 1, 0, sa, salen);	/* send 1-byte datagram */

	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL); //接受一个字节数据
	recvline[n] = '\0';	/* null terminate 接受缓冲区末尾加NULL*/
	Fputs(recvline, stdout); //把接受缓冲区打印到标准输出

	exit(0);
}
