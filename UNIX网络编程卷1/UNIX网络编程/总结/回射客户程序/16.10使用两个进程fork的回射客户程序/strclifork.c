#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	pid_t	pid;
	char	sendline[MAXLINE], recvline[MAXLINE];
	//创建一个子进程
	if ( (pid = Fork()) == 0) {		/* child: server -> stdout 在子进程中中,读取服务器的数据并发送到标准输出 */
		while (Readline(sockfd, recvline, MAXLINE) > 0) //从服务器读取数据,存放在recvline中
			Fputs(recvline, stdout); //把服务器中读取到的数据写到标准输出

		kill(getppid(), SIGTERM);	/* in case parent still running 
									   向父进程发送SIGTERM信号,终止父进程 */
		exit(0);
	}

		/* parent: stdin -> server 在父进程中,从标准输入读取,并发送到服务器 */
	while (Fgets(sendline, MAXLINE, fp) != NULL) //从标准输入fp读取数据,存放在sendline中
		Writen(sockfd, sendline, strlen(sendline)); //把从标准输入读取的数据,发送到服务器sockfd

	Shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN 关闭写端,想服务器发送fin */
	pause(); //等待子进程终止,向父进程发送SIGTERM信号来终止父进程
	return;
}
