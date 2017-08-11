/* include mcast_join1 */
#include	"unp.h"
#include	<net/if.h>

int
mcast_join(int sockfd, const SA *grp, socklen_t grplen,
		   const char *ifname, u_int ifindex)
{
#ifdef MCAST_JOIN_GROUP //如果指定了宏MCAST_JOIN_GROUP
	struct group_req req; 
	if (ifindex > 0) { //调用者给出了接口的索引
		req.gr_interface = ifindex; //把传参进来的接口索引填入req
	} else if (ifname != NULL) { //如果调用者给出了接口名字
		if ( (req.gr_interface = if_nametoindex(ifname)) == 0) { //把接口名转换成索引,并填入req
			errno = ENXIO;	/* i/f name not found 接口名没找到 */
			return(-1);
		}
	} else //既没有指定接口名,也没有指定接口索引
		req.gr_interface = 0; //以接口索引为0来分配任意匹配接口
	if (grplen > sizeof(req.gr_group)) { //给出的地址结构体长度太长
		errno = EINVAL; 
		return -1;
	}
	memcpy(&req.gr_group, grp, grplen); //把传参进来的多播地址填入reg中
	//用填写好的req来调用setsockopt设置MCAST_JOIN_GROUP套接字选项,来使套接字加入不限源的多播组
	return (setsockopt(sockfd, family_to_level(grp->sa_family),
			MCAST_JOIN_GROUP, &req, sizeof(req)));
#else //如果没有指定宏MCAST_JOIN_GROUP,把呢要根据不同的地址族来进行分别处理
/* end mcast_join1 */

/* include mcast_join2 */
	switch (grp->sa_family) { //确定地址族
	case AF_INET: { //IP4地址族:用接口的IP4地址,来表示,哪个接口加入多播组
		struct ip_mreq		mreq;
		struct ifreq		ifreq;
		//把传参进来的多播地址族,强转成IP4格式的地址
		//用IP4格式的地址,取出其中的sin_addr成员(地址成员)
		//把这个多播地址填入mreq结构体
		//至此mreq结构中,需要加入多播的地址填写完毕
		memcpy(&mreq.imr_multiaddr,
			   &((const struct sockaddr_in *) grp)->sin_addr,
			   sizeof(struct in_addr));

		if (ifindex > 0) { //如果调用者传参了接口索引
			//因为使ip4,所以接口索引要转换成接口名称,并填入ifreq结构体
			if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
				errno = ENXIO;	/* i/f index not found 接口索引没有找到*/
				return(-1);
			}
			goto doioctl;
		} else if (ifname != NULL) { //如果调用者指定了接口名称
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ); //把接口名称填入ifreq结构体
doioctl:
			//用填写好接口名字的ifreq结构体,调用ioctl来获得接口地址
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) //通过接口名字获得接口地址并填入ifreq结构体
				return(-1);
			//把ifreq结构体中获得的接口地址填入mreq结构体
			memcpy(&mreq.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else //调用者既没有指定接口名称,也没有指定接口索引
			   //那么就用INADDR_ANY任意指定一个接口,填入mreq结构体
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		//至此,需要加入多播的接口填写完毕(需要加入多播的IP4接口,用这个接口的地址来表示这个接口)

		return(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
						  &mreq, sizeof(mreq))); //调用setsockopt来加入多播组
	}
/* end mcast_join2 */

/* include mcast_join3 */
#ifdef	IPV6 //如果主机支持IP6协议
#ifndef	IPV6_JOIN_GROUP		/* APIv0 compatibility */
#define	IPV6_JOIN_GROUP		IPV6_ADD_MEMBERSHIP //定义宏IPV6_ADD_MEMBERSHIP
#endif
	case AF_INET6: { //IP6环境下,使用索引来表示需要加入多播组的接口
		struct ipv6_mreq	mreq6;
		//把传参进来的多播地址强转成IP6地址
		//取IP6地址结构体中的sin6_addr,填写入mreq6结构体
		memcpy(&mreq6.ipv6mr_multiaddr,
			   &((const struct sockaddr_in6 *) grp)->sin6_addr,
			   sizeof(struct in6_addr));

		if (ifindex > 0) { //如果调用者指定了接口索引
			mreq6.ipv6mr_interface = ifindex; //则把接口索引填入mreq6结构体
		} else if (ifname != NULL) { //如果调用者给出了接口名字
			if ( (mreq6.ipv6mr_interface = if_nametoindex(ifname)) == 0) { //则把接口名字转换成索引,填写入mreq6结构体
				errno = ENXIO;	/* i/f name not found 名字没有找到*/
				return(-1);
			}
		} else //如果既没有指定索引,也没有指定接口名称
			mreq6.ipv6mr_interface = 0; //则让内核自己选择匹配的接口
		//至此,mreq6结构体填写完毕

		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
						  &mreq6, sizeof(mreq6))); //把指定接口,加入指定多播组
	}
#endif

	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
#endif
}
/* end mcast_join3 */

void
Mcast_join(int sockfd, const SA *grp, socklen_t grplen,
		   const char *ifname, u_int ifindex)
{
	if (mcast_join(sockfd, grp, grplen, ifname, ifindex) < 0)
		err_sys("mcast_join error");
}

int
mcast_join_source_group(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen,
						const char *ifname, u_int ifindex)
{
#ifdef MCAST_JOIN_SOURCE_GROUP
	struct group_source_req req;
	if (ifindex > 0) {
		req.gsr_interface = ifindex;
	} else if (ifname != NULL) {
		if ( (req.gsr_interface = if_nametoindex(ifname)) == 0) {
			errno = ENXIO;	/* i/f name not found */
			return(-1);
		}
	} else
		req.gsr_interface = 0;
	if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
		errno = EINVAL;
		return -1;
	}
	memcpy(&req.gsr_group, grp, grplen);
	memcpy(&req.gsr_source, src, srclen);
	return (setsockopt(sockfd, family_to_level(grp->sa_family),
			MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req)));
#else
	switch (grp->sa_family) {
#ifdef IP_ADD_SOURCE_MEMBERSHIP
	case AF_INET: {
		struct ip_mreq_source	mreq;
		struct ifreq			ifreq;

		memcpy(&mreq.imr_multiaddr,
			   &((struct sockaddr_in *) grp)->sin_addr,
			   sizeof(struct in_addr));
		memcpy(&mreq.imr_sourceaddr,
			   &((struct sockaddr_in *) src)->sin_addr,
			   sizeof(struct in_addr));

		if (ifindex > 0) {
			if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
				errno = ENXIO;	/* i/f index not found */
				return(-1);
			}
			goto doioctl;
		} else if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
doioctl:
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
				return(-1);
			memcpy(&mreq.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);

		return(setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
						  &mreq, sizeof(mreq)));
	}
#endif

#ifdef	IPV6
	case AF_INET6: /* IPv6 source-specific API is MCAST_JOIN_SOURCE_GROUP */
#endif
	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
#endif
}

void
Mcast_join_source_group(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen,
						const char *ifname, u_int ifindex)
{
	if (mcast_join_source_group(sockfd, src, srclen, grp, grplen,
								ifname, ifindex) < 0)
		err_sys("mcast_join_source_group error");
}

int
mcast_block_source(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen)
{
#ifdef MCAST_BLOCK_SOURCE
	struct group_source_req req;
	req.gsr_interface = 0;
	if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
		errno = EINVAL;
		return -1;
	}
	memcpy(&req.gsr_group, grp, grplen);
	memcpy(&req.gsr_source, src, srclen);
	return (setsockopt(sockfd, family_to_level(grp->sa_family),
			MCAST_BLOCK_SOURCE, &req, sizeof(req)));
#else
	switch (grp->sa_family) {
#ifdef IP_BLOCK_SOURCE
	case AF_INET: {
		struct ip_mreq_source	mreq;

		memcpy(&mreq.imr_multiaddr,
			   &((struct sockaddr_in *) grp)->sin_addr,
			   sizeof(struct in_addr));
		memcpy(&mreq.imr_sourceaddr,
			   &((struct sockaddr_in *) src)->sin_addr,
			   sizeof(struct in_addr));
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);

		return(setsockopt(sockfd, IPPROTO_IP, IP_BLOCK_SOURCE,
						  &mreq, sizeof(mreq)));
	}
#endif

#ifdef	IPV6
	case AF_INET6: /* IPv6 source-specific API is MCAST_BLOCK_SOURCE */
#endif
	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
#endif
}

void
Mcast_block_source(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen)
{
	if (mcast_block_source(sockfd, src, srclen, grp, grplen) < 0)
		err_sys("mcast_block_source error");
}

int
mcast_unblock_source(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen)
{
#ifdef MCAST_UNBLOCK_SOURCE
	struct group_source_req req;
	req.gsr_interface = 0;
	if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
		errno = EINVAL;
		return -1;
	}
	memcpy(&req.gsr_group, grp, grplen);
	memcpy(&req.gsr_source, src, srclen);
	return (setsockopt(sockfd, family_to_level(grp->sa_family),
			MCAST_UNBLOCK_SOURCE, &req, sizeof(req)));
#else
	switch (grp->sa_family) {
#ifdef IP_UNBLOCK_SOURCE
	case AF_INET: {
		struct ip_mreq_source	mreq;

		memcpy(&mreq.imr_multiaddr,
			   &((struct sockaddr_in *) grp)->sin_addr,
			   sizeof(struct in_addr));
		memcpy(&mreq.imr_sourceaddr,
			   &((struct sockaddr_in *) src)->sin_addr,
			   sizeof(struct in_addr));
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);

		return(setsockopt(sockfd, IPPROTO_IP, IP_UNBLOCK_SOURCE,
						  &mreq, sizeof(mreq)));
	}
#endif

#ifdef	IPV6
	case AF_INET6: /* IPv6 source-specific API is MCAST_UNBLOCK_SOURCE */
#endif
	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
#endif
}

void
Mcast_unblock_source(int sockfd, const SA *src, socklen_t srclen,
						const SA *grp, socklen_t grplen)
{
	if (mcast_unblock_source(sockfd, src, srclen, grp, grplen) < 0)
		err_sys("mcast_unblock_source error");
}
