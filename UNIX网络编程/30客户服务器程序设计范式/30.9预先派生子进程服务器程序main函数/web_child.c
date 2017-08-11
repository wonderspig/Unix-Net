#include	"unp.h"

#define	MAXN	16384		/* max # bytes client can request */

void
web_child(int sockfd)
{
	int			ntowrite;
	ssize_t		nread;
	char		line[MAXLINE], result[MAXN];

	for ( ; ; ) {
		if ( (nread = Readline(sockfd, line, MAXLINE)) == 0) //从连接到客户端的套接字读取一行
			return;		/* connection closed by other end 读到EOF就返回,退出子进程*/

			/* 4line from client specifies #bytes to write back */
		ntowrite = atol(line); //line为客户端请求返回的字节数,把这个表达式格式转换成数值格式
		if ((ntowrite <= 0) || (ntowrite > MAXN)) //客户端传递的这个请求过大或者过小,就报错
			err_quit("client request for %d bytes", ntowrite);

		Writen(sockfd, result, ntowrite); //把客户端要求的字节数发送给客户端
	}
}
