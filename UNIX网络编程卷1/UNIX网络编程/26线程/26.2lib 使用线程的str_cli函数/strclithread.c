#include	"unpthread.h"

void	*copyto(void *);

static int	sockfd;		/* global for both threads to access */
static FILE	*fp;

void
str_cli(FILE *fp_arg, int sockfd_arg)
{
	char		recvline[MAXLINE];
	pthread_t	tid;

	sockfd = sockfd_arg;	/* copy arguments to externals 输入文件指针放入全局变量*/
	fp = fp_arg; //连接到服务器的套接字放入全局变量(供线程使用)

	Pthread_create(&tid, NULL, copyto, NULL); //创建线程,属性为NULL,线程函数参数为NULL

	while (Readline(sockfd, recvline, MAXLINE) > 0) //从套接字中读取数据,存放在recvline中
		Fputs(recvline, stdout); //把从服务器中读取来的数据发送到标准输出
}

void *
copyto(void *arg)
{
	char	sendline[MAXLINE];

	while (Fgets(sendline, MAXLINE, fp) != NULL) //从fp(标准输入)读
		Writen(sockfd, sendline, strlen(sendline)); //发送到连接到服务器的套接字

	Shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN 关闭写端,让主线程的读端继续读*/

	return(NULL);
		/* 4return (i.e., thread terminates) when EOF on stdin */
}
