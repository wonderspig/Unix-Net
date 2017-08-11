/* include tcp_listen */
#include	"unp.h"

int
tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_flags = AI_PASSIVE; //设置标志:作为监听套接字来获取地址
	hints.ai_family = AF_UNSPEC; //不指定ip4或者ip6
	hints.ai_socktype = SOCK_STREAM; //tcp

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) //获取地址结构体链表
		err_quit("tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res; //把链表浅复制一份

	//遍历链表循环尝试绑定地址
	do {
		//创建监听套接字
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one 失败就继续循环 */

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); //打开允许重用本地地址选项
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) //绑定地址
			break;			/* success 成功则直接跳出循环 */

		Close(listenfd);	/* bind error, close and try next one 关闭监听套接字继续循环 */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	/* errno from final socket() or bind() 说明获取地址失败 */
		err_sys("tcp_listen error for %s, %s", host, serv);

	Listen(listenfd, LISTENQ); //监听套接字,监听队列最大为LISTENQ

	if (addrlenp) //返回协议地址的大小
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave); //释放getaddrinfo分配的空间

	return(listenfd); //返回监听的套接字
}
/* end tcp_listen */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */

int
Tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(tcp_listen(host, serv, addrlenp));
}
