/* include host_serv */
#include	"unp.h"

struct addrinfo *
host_serv(const char *host, const char *serv, int family, int socktype)
{
	int				n;
	struct addrinfo	hints, *res;

	bzero(&hints, sizeof(struct addrinfo)); //初始化清空hints结构体
	hints.ai_flags = AI_CANONNAME;	/* always return canonical name 规范名称*/
	hints.ai_family = family;		/* AF_UNSPEC, AF_INET, AF_INET6, etc. 根据参数定义地址族*/
	hints.ai_socktype = socktype;	/* 0, SOCK_STREAM, SOCK_DGRAM, etc. 根据参数定义套接字类型 */

	//用穿餐进来的host,serv,且用设置好参数的hints,来调用getaddrinfo
	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) 
		return(NULL);

	return(res);	/* return pointer to first on linked list 返回调用getaddrinfo而填写的res */
}
/* end host_serv */

/*
 * There is no easy way to pass back the integer return code from
 * getaddrinfo() in the function above, short of adding another argument
 * that is a pointer, so the easiest way to provide the wrapper function
 * is just to duplicate the simple function as we do here.
 */

struct addrinfo *
Host_serv(const char *host, const char *serv, int family, int socktype)
{
	int				n;
	struct addrinfo	hints, *res;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;	/* always return canonical name */
	hints.ai_family = family;		/* 0, AF_INET, AF_INET6, etc. */
	hints.ai_socktype = socktype;	/* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("host_serv error for %s, %s: %s",
				 (host == NULL) ? "(no hostname)" : host,
				 (serv == NULL) ? "(no service name)" : serv,
				 gai_strerror(n)); //带出错处理的host_serv函数

	return(res);	/* return pointer to first on linked list */
}
