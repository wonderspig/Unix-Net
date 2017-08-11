#include	"unp.h"

void
str_echo(int sockfd)
{
	long		arg1, arg2;
	ssize_t		n;
	char		line[MAXLINE];

	for ( ; ; ) {
		if ( (n = Readline(sockfd, line, MAXLINE)) == 0) //从连接到客户的套接字读一行文本
			return;		/* connection closed by other end */

		if (sscanf(line, "%ld%ld", &arg1, &arg2) == 2) //把这一行文本中的两个数存入arg1,arg2中
			snprintf(line, sizeof(line), "%ld\n", arg1 + arg2); //line中保存两数相加结果
		else
			snprintf(line, sizeof(line), "input error\n"); //line中保存错误信息

		n = strlen(line); //计算当前line的长度
		Writen(sockfd, line, n); //写入连接到客户端的套接字
	}
}
