#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听套接字,ip4,TCP

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
	servaddr.sin_family      = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //填写通配地址
	servaddr.sin_port        = htons(SERV_PORT); //填写端口号

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //绑定已经填写好的地址结构体

	Listen(listenfd, LISTENQ); //监听这个套接字

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() 注册信号处理程序 */

	for ( ; ; ) {
		clilen = sizeof(cliaddr); //客户地址结构体长度
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) { //响应客户请求,并填写客户地址结构体
			if (errno == EINTR) //如果被信号中断,则重新启用
				continue;		/* back to for() */
			else
				err_sys("accept error"); //处理出错情况
		}

		if ( (childpid = Fork()) == 0) {	/* child process 在子进程中*/
			Close(listenfd);	/* close listening socket 关闭监听套接字 */
			str_echo(connfd);	/* process the request 在这个函数中处理客户请求 */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket 在父进程中关闭连接到子进程的套接字 */
	}
}
