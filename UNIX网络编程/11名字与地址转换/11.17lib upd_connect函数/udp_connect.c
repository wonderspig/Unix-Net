/* include udp_connect */
#include	"unp.h"

int
udp_connect(const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_family = AF_UNSPEC; //不规定IP地址族
	hints.ai_socktype = SOCK_DGRAM; //UDP

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) //根据主机名和服务名获得地址
		err_quit("udp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res; //把填写好的地址链表复制一份,用来释放资源

	//用循环遍历链表地址,创建套接字并连接
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //创建套接字
		if (sockfd < 0) //套接字创建失败,则继续循环
			continue;	/* ignore this one */
			//套接字创建成功
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) //则通过查找到的地址连接到这个套接字
			break;		/* success 成功则跳出循环*/

		Close(sockfd);	/* ignore this one 否则关闭这个套接字继续循环*/
	} while ( (res = res->ai_next) != NULL); //遍历链表中的地址直到NULL为止

	if (res == NULL)	/* errno set from final connect() 说明创建套接字和连接失败*/
		err_sys("udp_connect error for %s, %s", host, serv);

	freeaddrinfo(ressave); //释放getaddrinfo创建的链表资源

	return(sockfd);
}
/* end udp_connect */

int
Udp_connect(const char *host, const char *serv)
{
	int		n;

	if ( (n = udp_connect(host, serv)) < 0) {
		err_quit("udp_connect error for %s, %s: %s",
					 host, serv, gai_strerror(-n));
	}
	return(n);
}
