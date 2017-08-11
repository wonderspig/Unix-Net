#include	"unp.h"
//select会一直有一个异常条件准备好了,直到进程的读入越过了带外数据
//方式是读完带外数据后把标志量置1,并清除异常描述符集。
//等待正常读取数据之后(也就是越过了带外数据),再把标志量置0,从而重新将描述符加入异常描述符集

int
main(int argc, char **argv)
{
	int		listenfd, connfd, n, justreadoob = 0;
	char	buff[100];
	fd_set	rset, xset;

	if (argc == 2) //命令行1个参数
		//根据主机名和服务名搜索合适的地址,用这个地址的地址族和协议创建套接字,并绑定这个地址到套接字上面,然后监听这个套接字
		listenfd = Tcp_listen(NULL, argv[1], NULL);
	else if (argc == 3) //命令行2个参数
		//根据主机名和服务名搜索合适的地址,用这个地址的地址族和协议创建套接字,并绑定这个地址到套接字上面,然后监听这个套接字
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: tcprecv03 [ <host> ] <port#>");

	connfd = Accept(listenfd, NULL, NULL); //响应客户请求
 
	FD_ZERO(&rset); //清空读描述符集
	FD_ZERO(&xset); //清空写描述符集
	for ( ; ; ) {
		FD_SET(connfd, &rset); //把连接到客户端的套接字存放入读描述符集
		if (justreadoob == 0) //检查标志量(异常条件还没到达)
			FD_SET(connfd, &xset); //把连接到客户端的套接字加入到异常条件描述符集中

		Select(connfd + 1, &rset, NULL, &xset, NULL); //select描述符集

		if (FD_ISSET(connfd, &xset)) { //异常描述符集准备好了
			n = Recv(connfd, buff, sizeof(buff)-1, MSG_OOB); //读带外数据
			buff[n] = 0;		/* null terminate 缓冲区空字符结尾 */
			printf("read %d OOB byte: %s\n", n, buff); //打印带外数据内容
			justreadoob = 1; //标志量置1
			FD_CLR(connfd, &xset); //清除异常描述符集中连接到套接字的描述符
		}

		if (FD_ISSET(connfd, &rset)) { //读描述符集准备好了
			if ( (n = Read(connfd, buff, sizeof(buff)-1)) == 0) { //读取数据
				printf("received EOF\n");
				exit(0);
			}
			buff[n] = 0;	/* null terminate 缓冲区空字符结尾 */
			printf("read %d bytes: %s\n", n, buff); //打印读到的数据
			justreadoob = 0; //标志量置0
		}
	}
}
