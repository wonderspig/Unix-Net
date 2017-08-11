#include	"unp.h"

static void	recvfrom_alarm(int);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int				n;
	const int		on = 1;
	char			sendline[MAXLINE], recvline[MAXLINE + 1];
	socklen_t		len;
	struct sockaddr	*preply_addr;
 
	preply_addr = Malloc(servlen); //recvfrom返回的服务器地址分配空间

	Setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置广播套接字选项

	Signal(SIGALRM, recvfrom_alarm); //注册定时器SIGALRM信号

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入读入一行
		//把标准输入读入的数据发送到sockfd,地址为pservaddr
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen); 

		alarm(5); //5秒钟后收到SIGALRM信号
		for ( ; ; ) {
			len = servlen; //设置收到数据包对段的地址的长度
			//从套接字sockfd收消息,存放在recvline,地址记录在preply_addr中
			n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
			if (n < 0) { //被信号打断
				if (errno == EINTR)
					break;		/* waited long enough for replies 接收超时 */
				else
					err_sys("recvfrom error");
			} else {
				recvline[n] = 0;	/* null terminate 缓冲区以空字符结尾*/
				printf("from %s: %s",
						Sock_ntop_host(preply_addr, len), recvline); //打印接受数据包对端的主机信息
			}
		}
	}
	free(preply_addr);
}

static void
recvfrom_alarm(int signo)
{
	return;		/* just interrupt the recvfrom() */
}
