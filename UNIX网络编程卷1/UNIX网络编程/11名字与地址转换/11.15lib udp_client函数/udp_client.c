/* include udp_client */
#include	"unp.h"

int
udp_client(const char *host, const char *serv, SA **saptr, socklen_t *lenp)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_family = AF_UNSPEC; //不限定地址族
	hints.ai_socktype = SOCK_DGRAM; //UDP

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) //调用getaddrinfo搜索地址
		err_quit("udp_client error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res; //把搜索出来的地址复制一份(用来调用freeaddrinfo释放空间)

	//遍历链表中的地址来创建套接字
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd >= 0) //有一个成功就跳出循环
			break;		/* success */
	} while ( (res = res->ai_next) != NULL); //遍历链表直到空为止

	if (res == NULL)	/* errno set from final socket() 说明所有地址都无效,打印出错消息 */
		err_sys("udp_client error for %s, %s", host, serv);

	//为遍历到的地址成功创建套接字之后,为保存此地址的信息而动态分配空间
	*saptr = Malloc(res->ai_addrlen); 
	//深拷贝res结构体中的地址到用户传参进来的地址
	//此空间也是用户在函数外部动态分配好的
	memcpy(*saptr, res->ai_addr, res->ai_addrlen); 
	//为用户传参进来的存放地址结构体大小的参数附值
	*lenp = res->ai_addrlen;

	freeaddrinfo(ressave); //释放getaddrinfo函数的资源

	return(sockfd);
}
/* end udp_client */

int
Udp_client(const char *host, const char *serv, SA **saptr, socklen_t *lenptr)
{
	return(udp_client(host, serv, saptr, lenptr));
}
