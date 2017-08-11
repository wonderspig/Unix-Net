#include	"unp.h"

static void	recvfrom_int(int);
static int	count;

void
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	socklen_t	len;
	char		mesg[MAXLINE];

	Signal(SIGINT, recvfrom_int); //注册信号处理程序

	for ( ; ; ) {
		len = clilen; //客户端地址结构体的大小复制一份(用来调用Recvfrom)
		//送客户端sockfd出接受数据报,内容存放在mesg,客户端地址存放在pcliaddr中
		Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		count++; //每次收到依次数据报,则计数+1
	}
}

static void
recvfrom_int(int signo)
{
	printf("\nreceived %d datagrams\n", count); //当收到ctrl+c,则打印计数(已经收到了多少个数据报)
	exit(0);
}
