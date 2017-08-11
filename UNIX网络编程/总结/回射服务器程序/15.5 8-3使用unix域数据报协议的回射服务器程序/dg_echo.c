#include	"unp.h"

void
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	for ( ; ; ) {
		len = clilen; //客户端地址结构体的长度
		//从sockfd中读数据,存放入mesg,读MAXLINE字节,客户端地址存放在pcliaddr,客户端地址长度存放在len中
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len); 

		//把mesg中的内容发送到sockfd中,发送n个字节,发送到pcliaddr,地址长度为len
		Sendto(sockfd, mesg, n, 0, pcliaddr, len);
	}
}
