#include	"unp.h"

ssize_t	Dg_send_recv(int, const void *, size_t, void *, size_t,
				   const SA *, socklen_t);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	ssize_t	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		//Dg_send_recv函数代替了sendto函数和recvform函数
		n = Dg_send_recv(sockfd, sendline, strlen(sendline),
						 recvline, MAXLINE, pservaddr, servlen);

		recvline[n] = 0;	/* null terminate */
		Fputs(recvline, stdout);
	}
}
