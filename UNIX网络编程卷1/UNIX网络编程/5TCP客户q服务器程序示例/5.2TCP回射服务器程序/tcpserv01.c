#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建ip4,TCP套接字

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
	servaddr.sin_family      = AF_INET; //使用ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定任意地址
	servaddr.sin_port        = htons(SERV_PORT); //绑定端口号unp.h

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //把填写好的套接字,绑定在监听套接字上

	Listen(listenfd, LISTENQ); //监听这个套接字

	for ( ; ; ) {
		clilen = sizeof(cliaddr); //计算客户地址结构的长度
		connfd = Accept(listenfd, (SA *) &cliaddr, &clilen); //响应客户的请求

		if ( (childpid = Fork()) == 0) {	/* child process 创建子进程为客户服务*/
			Close(listenfd);	/* close listening socket 子进程中关闭监听套接字*/
			str_echo(connfd);	/* process the request 此函数处理回射数据 */
			exit(0); //退出子进程
		}
		Close(connfd);			/* parent closes connected socket 父进程中关闭连接到客户的套接字*/
	}
}
