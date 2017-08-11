#include	"unp.h"

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int		n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	//连接UDP套接字
	Connect(sockfd, (SA *) pservaddr, servlen); 

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入(fp)读,存入sendline中

		Write(sockfd, sendline, strlen(sendline)); //写到服务器(sockfd)中

		n = Read(sockfd, recvline, MAXLINE); //从服务器(sockfd)中读

		recvline[n] = 0;	/* null terminate 把缓存末尾加上'\0' */
		Fputs(recvline, stdout); //写到标准输出
	}
}
