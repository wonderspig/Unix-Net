#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int					listenfd, connfd; //监听套接字,连接套接字
	socklen_t			len; //套接字地址结构的长度
	struct sockaddr_in	servaddr, cliaddr; //套接字地址结构
	char				buff[MAXLINE];
	time_t				ticks;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听套接字,ip4,tcp

	bzero(&servaddr, sizeof(servaddr)); //清空结构体地址
	servaddr.sin_family      = AF_INET; //地址族为ip4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定任意地址
	servaddr.sin_port        = htons(13);	/* daytime server 把端口号填入*/

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //绑定填写好的地址

	Listen(listenfd, LISTENQ); //监听填写好的地址

	for ( ; ; ) {
		len = sizeof(cliaddr); //客户端地址结构的长度
		connfd = Accept(listenfd, (SA *) &cliaddr, &len); //响应客户端请求,获取客户端地址信息
		printf("connection from %s, port %d\n",
			   Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
			   ntohs(cliaddr.sin_port)); //打印客户地址结构体中的信息

        ticks = time(NULL); //获得当前日历时间
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); //格式化得到的日历时间,存入buff中
        Write(connfd, buff, strlen(buff)); //把结果吸入连接客户端的套接字中

		Close(connfd);
	}
}
