#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int listenfd, connfd;
	socklen_t addrlen, len;
	struct sockaddr	*cliaddr;
	char buff[MAXLINE];
	time_t ticks;

	if (argc < 2 || argc > 3) //在初始化为守护进程之前,检查命令行参数
		err_quit("usage: daytimetcpsrv2 [ <host> ] <service or port>");

	daemon_init(argv[0], 0); //初始化为守护进程

	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);

	cliaddr = Malloc(addrlen);

	for ( ; ; ) {
		len = addrlen;
		connfd = Accept(listenfd, cliaddr, &len);
		//不再用printf打印消息，改称往日志文件中写
		err_msg("connection from %s", Sock_ntop(cliaddr, len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Write(connfd, buff, strlen(buff));

		Close(connfd);
	}
}
