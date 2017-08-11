#include	"unp.h"

int
main(int argc, char **argv)
{
	int		listenfd, connfd, n, on=1;
	char	buff[100];

	if (argc == 2) //命令行1个参数
		//查找地址,并创建套接字,然后绑定这个地址,并监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], NULL);
	else if (argc == 3) //命令行2个参数
		//查找地址,并创建套接字,然后绑定这个地址,并监听这个套接字
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: tcprecv04 [ <host> ] <port#>");
	//设置套接字选项SO_OOBINLINE,此选项会在线接收套接字
	Setsockopt(listenfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on));

	connfd = Accept(listenfd, NULL, NULL); //响应客户请求
	sleep(5); 

	for ( ; ; ) {
		if (Sockatmark(connfd)) //查看是否到达带外标记
			printf("at OOB mark\n");

		if ( (n = Read(connfd, buff, sizeof(buff)-1)) == 0) { //读数据
			printf("received EOF\n");
			exit(0);
		}
		buff[n] = 0;	/* null terminate 缓冲区空字符结尾 */
		printf("read %d bytes: %s\n", n, buff); //打印读到的数据
	}
}
