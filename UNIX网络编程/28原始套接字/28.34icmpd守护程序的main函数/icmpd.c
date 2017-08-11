/* include icmpd1 */
#include	"icmpd.h"

int
main(int argc, char **argv)
{
	int		i, sockfd;
	struct sockaddr_un sun;

	if (argc != 1) //如果命令行参数不是两个,则出错
		err_quit("usage: icmpd");

	maxi = -1;//client结构体数组中已经被使用的元素,初始化为-1			/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++) //循环遍历client结构体数组的每一个元素
		//每个元素的connd成员(连接到客户端的Unix域套接字)初始化为-1,代表这个成员没有被使用
		client[i].connfd = -1;	/* -1 indicates available entry */
	FD_ZERO(&allset); //清空allset描述符集

	fd4 = Socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //创建一个原始套接字(IP4地址族,协议为ICMP)
	FD_SET(fd4, &allset); //把这个ICMP4的原始套接字,加入描述符集中
	maxfd = fd4; //最大描述符集设置为这个ICMP4原始套接字

#ifdef	IPV6
	fd6 = Socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6); //创建一个原始套接字(IP6地址族,协议为ICMP)
	FD_SET(fd6, &allset); //把这个ICMP4的原始套接字,加入描述符集中
	maxfd = max(maxfd, fd6);//最大描述符集设置为ICMP4和ICMP6中较大的一个
#endif

	listenfd = Socket(AF_UNIX, SOCK_STREAM, 0); //创建一个Unix域套接字,用来监听客户进程的请求
	sun.sun_family = AF_LOCAL; //地址族为Unix域地址族
	strcpy(sun.sun_path, ICMPD_PATH); //设置Unix域地址结构体中的路径成员
	unlink(ICMPD_PATH); //删除这个路径所代表的文件(引用计数-1)
	Bind(listenfd, (SA *)&sun, sizeof(sun)); //帮这个设置好的Unix域地址结构体绑定到这个UNix域套接字中
	Listen(listenfd, LISTENQ); //监听这个Unix域套接字
	FD_SET(listenfd, &allset); //把这个监听Unix域套接字也加入描述符集中
	maxfd = max(maxfd, listenfd); //设置Select需要使用额最大描述符集
/* end icmpd1 */

/* include icmpd2 */
	//程序主循环
	for ( ; ; ) { 
		rset = allset; //把设置好的allset描述符集复制一份到rset,传入Select的是rset描述符集
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL); //用Select关系刚才设置的描述符集中,那些描述符可读

		if (FD_ISSET(listenfd, &rset)) //如果是监听描述符可读(有新的客户到来)
			if (readable_listen() <= 0) //调用readable_listen函数进行处理,处理完了nready变量-1
										//readable_listen函数: accept描述符,并把连接到客户的Unix套接字存入client结构体数组中
				//Select所返回的已准备好的描述符都处理完了,就重新还是循环,继续Select
				continue;

		if (FD_ISSET(fd4, &rset)) //如果是ICMP4原始套接字描述符准备好了
			if (readable_v4() <= 0) //调用readable_v4函数进行处理,处理完了nready变量-1
									//readable_v4函数:把内核接收到的ICMP数据报,组成icmpd_err结构体,返回给客户
				//Select所返回的已准备好的描述符都处理完了,就重新还是循环,继续Select
				continue; 

#ifdef	IPV6
		if (FD_ISSET(fd6, &rset)) //如果是ICMP6原始套接字描述符准备好了
			if (readable_v6() <= 0) //调用readable_v6函数进行处理,处理完了nready变量-1
									////readable_v6函数:把内核接收到的ICMP数据报,组成icmpd_err结构体,返回给客户
				//Select所返回的已准备好的描述符都处理完了,就重新还是循环,继续Select
				continue;
#endif
		//循环遍历client数组,maxi为当前client结构体数组中被使用元素最多的时候,下标所达到的最大值
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i].connfd) < 0) //client[i].connfd说明当前成员未被使用
				continue; //继续循环遍历
			//i索引所代表的client结构体数组成员正在被使用
			if (FD_ISSET(sockfd, &rset)) //i索引所代表的client结构体数组成员发了一条请求过来
				if (readable_conn(i) <= 0) //调用readable_conn函数进行处理,处理完了nready变量-1
											//readable_conn函数:读取客户发来的请求,根据索引i找到对应的client结构体数组元素,补全这个元素的其他成员信息
																//(如客户传递过来的描述符,绑定的端口号等等)
					break;				/* no more readable descriptors */
		}
	}
	exit(0);
}
/* end icmpd2 */
