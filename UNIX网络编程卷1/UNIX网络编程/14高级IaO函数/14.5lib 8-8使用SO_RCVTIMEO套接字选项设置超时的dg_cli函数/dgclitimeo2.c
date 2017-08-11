#include	"unp.h"

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int				n;
	char			sendline[MAXLINE], recvline[MAXLINE + 1];
	struct timeval	tv;

	//设置时间结构体为5秒
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	Setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); //设置套接字选项

	while (Fgets(sendline, MAXLINE, fp) != NULL) {

		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				fprintf(stderr, "socket timeout\n");
				continue;
			} else
				err_sys("recvfrom error");
		}

		recvline[n] = 0;	/* null terminate */
		Fputs(recvline, stdout);
	}
}
