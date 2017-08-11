/* include udp_server */
#include	"unp.h"

int
udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_flags = AI_PASSIVE; //作为被动打开的套接字
	hints.ai_family = AF_UNSPEC; //不关心地址族
	hints.ai_socktype = SOCK_DGRAM; //udp

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) //获得地址的链表
		err_quit("udp_server error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res; //把获得的地址复制一份,用来释放资源

	//遍历链表的地址,创建套接字并绑定地址
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //创建套接字
		if (sockfd < 0) //创建套接字失败则继续循环
			continue;		/* error - try next one */

		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0) //绑定套接字
			break;			/* success 成功则退出循环 */

		Close(sockfd);		/* bind error - close and try next one 失败则关闭这个套接字继续循环 */
	} while ( (res = res->ai_next) != NULL); //遍历链表直到末尾

	if (res == NULL)	/* errno from final socket() or bind() 如果创建套接字和绑定失败 */
		err_sys("udp_server error for %s, %s", host, serv);

	if (addrlenp) //如果调用着需要直到地址结构体的长度,则返回这个值
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave); //释放getaddrinfo所分配的资源

	return(sockfd);
}
/* end udp_server */

int
Udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(udp_server(host, serv, addrlenp));
}
