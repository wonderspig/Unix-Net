#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1;
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE];

	FD_ZERO(&rset); //清空描述符集
	for ( ; ; ) {
		FD_SET(fileno(fp), &rset); //把文件指针转换成文件描述符后设置进rset
		FD_SET(sockfd, &rset); //把sockfd设置进rset
		maxfdp1 = max(fileno(fp), sockfd) + 1; //取两个描述符最大的那个描述符然后+1
		Select(maxfdp1, &rset, NULL, NULL, NULL); //调用select,等待其中一个描述符准备好

		if (FD_ISSET(sockfd, &rset)) {	/* socket is readable 如果是sockfd准备好了 */
			if (Readline(sockfd, recvline, MAXLINE) == 0) //从sockfd中读取一行,遇到'\n',或EOF
				err_quit("str_cli: server terminated prematurely");
			Fputs(recvline, stdout); //写到标准输出
		}

		if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable 如果是fp(标准输入有数据)准备好了*/
			if (Fgets(sendline, MAXLINE, fp) == NULL) //从fp(标准输入中读取数据)
				return;		/* all done */
			Writen(sockfd, sendline, strlen(sendline));//写到sockfd中
		}
	}
}
