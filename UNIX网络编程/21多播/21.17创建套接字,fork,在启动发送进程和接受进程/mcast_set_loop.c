/* include mcast_set_loop */
#include	"unp.h"

int
mcast_set_loop(int sockfd, int onoff)
{
	switch (sockfd_to_family(sockfd)) { //获取套接字的地址族
	case AF_INET: { //IP4地址族
		u_char		flag;

		flag = onoff; //传参进来的标志复制一份
		//用IP4地址族去调用setsockopt设置标志
		return(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
						  &flag, sizeof(flag)));
	}

#ifdef	IPV6
	case AF_INET6: { //ip1地址族
		u_int		flag;

		flag = onoff;
		//用IP6地址族去调用setsockopt设置标志
		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
						  &flag, sizeof(flag)));
	}
#endif

	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
}
/* end mcast_set_loop */

void
Mcast_set_loop(int sockfd, int onoff)
{
	if (mcast_set_loop(sockfd, onoff) < 0)
		err_sys("mcast_set_loop error");
}
