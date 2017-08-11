#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入读一行

		Writen(sockfd, sendline, strlen(sendline)); //把从标准输入读到的一行写入套接字,传送给服务器

		if (Readline(sockfd, recvline, MAXLINE) == 0) //从套接字读取一行文本行,到'\n'为止
			err_quit("str_cli: server terminated prematurely");

		Fputs(recvline, stdout); //把从套接字读到的一行压入标准输出
	}
}
