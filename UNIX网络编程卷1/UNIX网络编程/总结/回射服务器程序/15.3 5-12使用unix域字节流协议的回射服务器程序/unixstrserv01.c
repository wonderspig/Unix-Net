#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_un	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = Socket(AF_LOCAL, SOCK_STREAM, 0); //创建监听套接字,unix套接字,tcp

	unlink(UNIXSTR_PATH); //删除众所周知的路径文件
	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sun_family = AF_LOCAL; //地址族为unix域套接字
	strcpy(servaddr.sun_path, UNIXSTR_PATH); //填写地址结构体中的路径名

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //通过填写好的地址结构体,绑定地址

	Listen(listenfd, LISTENQ); //监听这个套接字

	Signal(SIGCHLD, sig_chld); //注册SIGCHLD子进程结束的信号处理函数

	for ( ; ; ) {
		clilen = sizeof(cliaddr); //计算一下对端(客户端地址)的结构体
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) { //调用accept链接客户端,填写地址
			if (errno == EINTR) //如果被信号打断，则继续
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}
		//创建一个子进程
		if ( (childpid = Fork()) == 0) {	/* child process 再紫禁城中*/
			Close(listenfd);	/* close listening socket 关闭监听套接字*/
			str_echo(connfd);	/* process request 执行服务器程序的步骤 */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
