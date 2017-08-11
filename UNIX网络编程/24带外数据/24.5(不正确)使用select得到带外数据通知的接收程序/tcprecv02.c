#include	"unp.h"

int
main(int argc, char **argv)
{
	int		listenfd, connfd, n;
	char	buff[100];
	fd_set	rset, xset;

	if (argc == 2) //命令行一个参数
		//根据主机名和服务名搜索合适的地址,用这个地址的地址族和协议创建套接字,并绑定这个地址到套接字上面,然后监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], NULL);
	else if (argc == 3) //命令行两个参数
		//根据主机名和服务名搜索合适的地址,用这个地址的地址族和协议创建套接字,并绑定这个地址到套接字上面,然后监听这个套接字
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: tcprecv02 [ <host> ] <port#>");

	connfd = Accept(listenfd, NULL, NULL); //响应客户请求

	FD_ZERO(&rset); //清空读描述符集
	FD_ZERO(&xset); //清空写描述符集
	for ( ; ; ) {
		FD_SET(connfd, &rset); //把连接到客户端的套接字加入读描述符集
		FD_SET(connfd, &xset); //把连接到客户端的套接字加入异常描述符集

		Select(connfd + 1, &rset, NULL, &xset, NULL); //select描述符集

		if (FD_ISSET(connfd, &xset)) { //如果是异常状态有数据(带外数据到了)
			n = Recv(connfd, buff, sizeof(buff)-1, MSG_OOB); //接收带外数据
			buff[n] = 0;		/* null terminate 缓冲区空字符结尾 */
			printf("read %d OOB byte: %s\n", n, buff); //打印接收到的带外数据
		}

		if (FD_ISSET(connfd, &rset)) { //读描述符集准备好了
			if ( (n = Read(connfd, buff, sizeof(buff)-1)) == 0) { //读取普通数据
				printf("received EOF\n"); 
				exit(0);
			}
			buff[n] = 0;	/* null terminate 缓冲区空字符结尾 */
			printf("read %d bytes: %s\n", n, buff); //打印读到的数据
		}
	}
}
