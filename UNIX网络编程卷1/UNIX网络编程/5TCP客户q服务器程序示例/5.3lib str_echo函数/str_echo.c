#include	"unp.h"

void
str_echo(int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE];

again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0) //从连接到客户的套接字中读
		Writen(sockfd, buf, n); //写到连接到客户的套接字

	if (n < 0 && errno == EINTR) //如果read是被信号打断的
		goto again; //重新调用read
	else if (n < 0) //未知错误处理
		err_sys("str_echo: read error");
}
