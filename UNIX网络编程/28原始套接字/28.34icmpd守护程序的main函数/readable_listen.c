#include	"icmpd.h"

int
readable_listen(void)
{
	int			i, connfd;
	socklen_t	clilen;
	
	//进入此函数说明有新的客户到来,此时accept不会阻塞,会立刻返回

	clilen = sizeof(cliaddr); //Unix域地址结构体的大小
	connfd = Accept(listenfd, (SA *)&cliaddr, &clilen); //获得连接客户端的套接字connfd

		/* 4find first available client[] structure */
	for (i = 0; i < FD_SETSIZE; i++) //遍历整个client结构体数组(此数组的每一个元素保存一个客户进程的信息)
		if (client[i].connfd < 0) { //查找没有被使用的元素
			//使用这个元素
			client[i].connfd = connfd;	/* save descriptor 把连接到客户的描述符保存到client数组元素的对应成员中*/
			break;
		}
	if (i == FD_SETSIZE) { //此判断式表明搜索整个client结构体数组,也没有找到一个可用的client数组元素
		//说明服务器负荷已满
		close(connfd);		/* can't handle new client, 那就关闭这个已经连接到客户的套接字*/
		return(--nready);	/* rudely close the new connection 待处理的描述符个数-1,函数返回*/
	}
	//代码走到此处,说明已经找到了可用的client数组元素,且成功把连接到客户的Unix域套接字保存到其中
	printf("new connection, i = %d, connfd = %d\n", i, connfd); //打印这个客户进程的相关信息

	FD_SET(connfd, &allset);	/* add new descriptor to set 把连接到这个客户的Unix域套接字,加入Select的描述符集中*/
	if (connfd > maxfd)
		maxfd = connfd;			/* for select() 维护更新Select所使用的最大描述符号*/
	if (i > maxi)
		maxi = i;				/* max index in client[] array 维护更新client结构体数组元素被使用最多的时候,下标所达到的最大值*/

	return(--nready); //待处理的描述符个数-1,函数返回
}
