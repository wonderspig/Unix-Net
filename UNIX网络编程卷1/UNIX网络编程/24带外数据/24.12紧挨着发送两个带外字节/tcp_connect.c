/* include tcp_connect */
#include	"unp.h"

int
tcp_connect(const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_family = AF_UNSPEC; //不限定ip4或ip6
	hints.ai_socktype = SOCK_STREAM; //tcp

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) //调用getaddrinfo获取地址
		err_quit("tcp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res; //把getaddrinfo填写的结构复制一份

	//遍历链表里的地址来连接
	do {
		//建立套接字
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) //失败则继续循环
			continue;	/* ignore this one */

		//连接这个搜索到的地址
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) 
			break;		/* success 成功则跳出循环 */

		Close(sockfd);	/* ignore this one 失败则关闭套接字后继续循环*/
	} while ( (res = res->ai_next) != NULL); //遍历链表直到下一个为空为止

	if (res == NULL)	/* errno set from final connect() */
		err_sys("tcp_connect error for %s, %s", host, serv);

	freeaddrinfo(ressave); //释放getaddrinfo动态分配的空间

	return(sockfd); //返回这个连接成功的套接字
}
/* end tcp_connect */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
 */

int
Tcp_connect(const char *host, const char *serv)
{
	return(tcp_connect(host, serv));
}
