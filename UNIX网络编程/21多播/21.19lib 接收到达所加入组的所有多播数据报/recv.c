#include	"unp.h"

void
recv_all(int recvfd, socklen_t salen)
{
	int					n;
	char				line[MAXLINE+1];
	socklen_t			len;
	struct sockaddr		*safrom;

	safrom = Malloc(salen); //分配一个套接字地址结构(此地址结构的长度为发送端目的多播地址的长度)

	for ( ; ; ) {
		len = salen;
		n = Recvfrom(recvfd, line, MAXLINE, 0, safrom, &len); //从套接字接收数据

		line[n] = 0;	/* null terminate 接收缓冲区缓冲区null字符结尾 */
		printf("from %s: %s", Sock_ntop(safrom, len), line); //打印缓冲区信息
		//Sock_ntop函数:协议无关的把二进制地址转换成表达式
	}
}
