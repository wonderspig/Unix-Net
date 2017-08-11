/* include udpservselect01 */
#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd, udpfd, nready, maxfdp1;
	char				mesg[MAXLINE];
	pid_t				childpid;
	fd_set				rset;
	ssize_t				n;
	socklen_t			len;
	const int			on = 1;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

		/* 4create listening TCP socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听套接字,ip4,TCP

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family      = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配ip地址
	servaddr.sin_port        = htons(SERV_PORT); //绑定端口号

	//设置套接字选项(SO_REUSEADDR),有了这个选项就既可以绑定tcp,又可以绑定udp
	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); 
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //把这个填写好的地址绑定到监听套接字上

	Listen(listenfd, LISTENQ); //监听这个套接字

		/* 4create UDP socket */
	udpfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建一个ip4的UDP套接字

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family      = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配ip地址
	servaddr.sin_port        = htons(SERV_PORT); //绑定端口号

	Bind(udpfd, (SA *) &servaddr, sizeof(servaddr)); //把填写好的地址绑定到udp套接字上
/* end udpservselect01 */

/* include udpservselect02 */
	Signal(SIGCHLD, sig_chld);	/* must call waitpid() 注册信号处理程序(处理子进程结束信号) */

	FD_ZERO(&rset); //清空rset描述符集
	maxfdp1 = max(listenfd, udpfd) + 1; //最大描述符(select的参数)
	for ( ; ; ) {
		FD_SET(listenfd, &rset); //把监听套接字设置进描述符集中
		FD_SET(udpfd, &rset); //把udp套接字设置进描述符集中
		//用select等待监听套接字和和udp套接字其中之一准备好
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR) //如果被信号打断,则重启select
				continue;		/* back to for() */
			else
				err_sys("select error"); //否则打印错误消息
		}

		if (FD_ISSET(listenfd, &rset)) { //判断如果是监听套接字准备好了
			len = sizeof(cliaddr); //求一下客户端地址的长度(用来填写Accept函数的参数)
			//响应连接客户端,并填写客户端地址,获得连接到客户端的socket
			connfd = Accept(listenfd, (SA *) &cliaddr, &len); 
	
			//创建一个子进程,在子进程中
			if ( (childpid = Fork()) == 0) {	/* child process 在子进程中*/
				Close(listenfd);	/* close listening socket 关闭监听套接字*/
				str_echo(connfd);	/* process the request 在这个函数中响应客户端请求 */
				exit(0);
			}
			Close(connfd);			/* parent closes connected socket 父进程中关闭连接到客户端的套接字*/
		}

		if (FD_ISSET(udpfd, &rset)) { //判断是不是udp套接字准备好了
			len = sizeof(cliaddr); ////求一下客户端地址的长度(用来填写Recvfrom函数的参数)
			//从udp套接字中接受数据,存放在mesg中,客户端的地址填写在cliaddr中
			n = Recvfrom(udpfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len); 

			//把mesg中的数据发送还给udp套接字
			Sendto(udpfd, mesg, n, 0, (SA *) &cliaddr, len); 
		}
	}
}
/* end udpservselect02 */
