/* include fig01 */
#include	"unp.h"

int
main(int argc, char **argv)
{
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听套接字,ip4,tcp

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构
	servaddr.sin_family      = AF_INET; //地址族ip4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //填入通配地址
	servaddr.sin_port        = htons(SERV_PORT); //填入端口号

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //绑定填写好的地址结构到套接字上

	Listen(listenfd, LISTENQ); //监听套接字

	maxfd = listenfd;			/* initialize 当前select的最大套接字为listenfd+1*/
	maxi = -1;					/* index into client[] array 初始化client数组的下标 */
	for (i = 0; i < FD_SETSIZE; i++) //把client数组的所有成员都初始化为-1
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset); //初始化信号集,清空
	FD_SET(listenfd, &allset); //设置监听套接字到信号集中
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		rset = allset;		/* structure assignment 把信号集复制一份用来填入select */
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL); //调用select来阻塞等到,关心描述符可读

		//当select返回后,判断是否是监听套接字准备好了
		if (FD_ISSET(listenfd, &rset)) {	/* new client connection 代表一个新的客户请求到达了*/
			clilen = sizeof(cliaddr); //计算一下客户地址结构的长度
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen); //响应客户请求,填写客户地址结构体
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port)); //打印客户的地址信息
#endif

			for (i = 0; i < FD_SETSIZE; i++) //用循环遍历客户数组,查找为-1的项目,为-1的项目可以用来存放新客户的描述符
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor 找到了,就把新连接成功的客户描述符放入client数组 */
					break; 
				}
			if (i == FD_SETSIZE) //如果客户过多,则打印出错消息
				err_quit("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set 把连接到客户的文件描述符加入到select的关心描述符中*/
			if (connfd > maxfd)
				maxfd = connfd;			/* for select //更新最大文件描述符数(也是调用select的参数)*/
			if (i > maxi)
				maxi = i;				/* max index in client[] array 更新保存客户连接描述符数组的最大索引数 */

			if (--nready <= 0)
				continue;				/* no more readable descriptors 就绪的描述符判断 */
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data 遍历整个保存连接到客户的描述符的数组*/
			//如果是-1,说明这一位没有保存连接到客户的文件描述符
			if ( (sockfd = client[i]) < 0) 
				continue; //继续循环
			if (FD_ISSET(sockfd, &rset)) { //依次把找到的client数组中的有效文件描述符与rset判断,看看是否准备好
				//进入分支代表这个描述符准备好了
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) { //准备好了就去读,存放入buf中
						/*4connection closed by client 进入此分支代表读到EOF,客户套接字的请求已经写完了 */
					Close(sockfd); //关闭连接到客户的套接字
					FD_CLR(sockfd, &allset); //把这个客户的套接字从套接字集中删除
					client[i] = -1; //保存客户套接字描述符的数组中响应位删除
				} else
					Writen(sockfd, buf, n); //如果客户没有EOF,则提供回射服务,把读到的数据写回去

				if (--nready <= 0) //就绪的描述符判断
					break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */
