#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从fp(标准输入读)

		Writen(sockfd, sendline, 1); //写到连接到服务器的套接字
		sleep(1); //睡一秒
		Writen(sockfd, sendline+1, strlen(sendline)-1); //再写一次

		if (Readline(sockfd, recvline, MAXLINE) == 0) //从连接到服务器的套接字读一行,到'\n'为止
			err_quit("str_cli: server terminated prematurely");

		Fputs(recvline, stdout); //把读到的内容写到标准输出
	}
}
