#include	"unp.h"

int
main(int argc, char **argv)
{
	int				sockfd, n;
	char			recvline[MAXLINE + 1];
	socklen_t		len;
	struct sockaddr_storage	ss;

	if (argc != 3) //如果参数不为2,则出错退出
		err_quit("usage: daytimetcpcli <hostname/IPaddress> <service/port#>");

	sockfd = Tcp_connect(argv[1], argv[2]); //用参数1为主机名,参数2为服务名,查找地址并连接

	len = sizeof(ss); //计算新的通用套接字地址结构的大小
	Getpeername(sockfd, (SA *)&ss, &len); //获得sockfd对端地址,填写在ss中
	//打印已经连接到的主机
	printf("connected to %s\n", Sock_ntop_host((SA *)&ss, len)); //把地址中的主机部分,转换成表达式

	while ( (n = Read(sockfd, recvline, MAXLINE)) > 0) { //从服务器套接字sockfd中读
		recvline[n] = 0;	/* null terminate 末尾添加NULL字节 */
		Fputs(recvline, stdout); //打印到标准输出
	}
	exit(0);
}
