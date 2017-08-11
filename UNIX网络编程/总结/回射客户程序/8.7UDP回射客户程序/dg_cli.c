#include	"unp.h"

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入(fp)读入一行,存放入sendline

		//发送给sockfd(服务器),发送内容为sendline,发送地址为pservaddr,地址结构的长度为servlen
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen); 

		//从sockfd(服务器)接受消息,存放入recvline,并且对服务器的地址不感兴趣
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

		recvline[n] = 0;	/* null terminate 让字符串以'\0'结尾 */
		Fputs(recvline, stdout); //把从服务器读到的数据(recvline),写到标准输出
	}
}
