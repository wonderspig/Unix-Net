#include	"unp.h"

static void	sig_alrm(int);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	Signal(SIGALRM, sig_alrm); //注册sig_alrm的信号处理函数

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入fp读入一行

		//发送从标准输入读入的sendline到服务器sockfd
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		alarm(5); //设定alarm信号在5秒中之后发送
		//从服务器sockfd中读取数据存放在recvline中(最多5秒就会被打断)
		if ( (n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
			if (errno == EINTR) //这个错误代表是被打断的
				fprintf(stderr, "socket timeout\n");
			else
				err_sys("recvfrom error");
		} else { //代表recvfrom正常接收到数据
			alarm(0); //取消alarm信号
			recvline[n] = 0;	/* null terminate 缓冲区末尾加上NULL */
			Fputs(recvline, stdout); //把处理好的接受缓冲区打印到标准输出
		}
	}
}

static void
sig_alrm(int signo) //alarm的信号处理函数直接返回
{
	return;			/* just interrupt the recvfrom() */
}
