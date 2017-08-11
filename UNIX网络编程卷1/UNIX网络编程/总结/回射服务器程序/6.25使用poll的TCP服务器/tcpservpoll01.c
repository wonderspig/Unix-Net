/* include fig01 */
#include	"unp.h"
#include	<limits.h>		/* for OPEN_MAX */

int
main(int argc, char **argv)
{
	int					i, maxi, listenfd, connfd, sockfd;
	int					nready;
	ssize_t				n;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct pollfd		client[OPEN_MAX];
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听套接字

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构数组
	servaddr.sin_family      = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配地址
	servaddr.sin_port        = htons(SERV_PORT); //填写端口号

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //绑定填写好的地址结构到监听套接字

	Listen(listenfd, LISTENQ); //监听套接字

	//poll函数的第一个参数是一个结构体数组,这次不再创建一个另外的client数组了,直接对这个poll的第一个参数进行操作
	//client数组是poll函数的第一个参数

	client[0].fd = listenfd; //把监听套接字放在client数组的第一个为止
	client[0].events = POLLRDNORM; //关心普通数据可读
	for (i = 1; i < OPEN_MAX; i++) //把数组中的每一个参数的fd都初始化为-1,除了第一个之外(已经存放了监听套接字)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array 数组最大下标为0,只存放了一个监听套接字*/
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		nready = Poll(client, maxi+1, INFTIM); //再无限循环中调用poll关心数组中的文件描述符

		if (client[0].revents & POLLRDNORM) {	/* new client connection 如果监听套接字的普通数据可读*/
			clilen = sizeof(cliaddr); //计算客户端地址结构的长度
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen); //与客户端建立连接
#ifdef	NOTDEF
			printf("new client: %s\n", Sock_ntop((SA *) &cliaddr, clilen)); //打印客户端地址信息
#endif

			for (i = 1; i < OPEN_MAX; i++) //循环遍历数组
				if (client[i].fd < 0) { //查找出数组中没有使用的还未存放fd的成员
					client[i].fd = connfd;	/* save descriptor 把新建立的连接到客户端的fd存入poll的数组中*/
					break;
				}
			if (i == OPEN_MAX) //表示最大描述符已经到上限
				err_quit("too many clients");

			client[i].events = POLLRDNORM; //这个连接好的描述符关心普通数据可读
			if (i > maxi)
				maxi = i;				/* max index in client[] array 更新数组的最大下标 */

			if (--nready <= 0) //看看符合条件的描述符是否处理完了
				continue;				/* no more readable descriptors */
		}

		for (i = 1; i <= maxi; i++) {	/* check all clients for data 遍历这个客户端数组 */
			if ( (sockfd = client[i].fd) < 0) //遍历查找出所有fd不为-1的书组成员
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) { //如果是普通数据可读,或者出现错误
				if ( (n = read(sockfd, buf, MAXLINE)) < 0) { //调用read看看到底是什么情况
					if (errno == ECONNRESET) { //客户端已经终止
							/*4connection reset by client */
#ifdef	NOTDEF
						printf("client[%d] aborted connection\n", i); //打印忽略客户连接
#endif
						Close(sockfd); //关闭这个客户端套接字
						client[i].fd = -1; //从poll数组中删除这个套接字
					} else
						err_sys("read error"); //其他错误
				} else if (n == 0) { //如果从客户端读到EOF
						/*4connection closed by client */
#ifdef	NOTDEF
					printf("client[%d] closed connection\n", i); //打印客户端已关闭
#endif
					Close(sockfd); //关闭客户端套接字
					client[i].fd = -1; //从poll数组中删除这个套接字
				} else
					Writen(sockfd, buf, n); //正常读到数据的话,就把读到的数据写入连接客户端的fd

				if (--nready <= 0) //没有更多的描述符待处理
					break;			/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */
