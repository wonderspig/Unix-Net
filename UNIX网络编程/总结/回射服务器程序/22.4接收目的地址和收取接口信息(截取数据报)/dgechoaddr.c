#include	"unpifi.h"

#undef	MAXLINE
#define	MAXLINE	20		/* to see datagram truncation */

void
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int						flags;
	const int				on = 1;
	socklen_t				len;
	ssize_t					n;
	char					mesg[MAXLINE], str[INET6_ADDRSTRLEN],
							ifname[IFNAMSIZ];
	struct in_addr			in_zero;
	struct unp_in_pktinfo	pktinfo;

#ifdef	IP_RECVDSTADDR //IP_RECVDSTADDR宏代表获得发送端的地址
	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on)) < 0)
		err_ret("setsockopt of IP_RECVDSTADDR");
#endif
#ifdef	IP_RECVIF //IP_RECVIF宏代表获得发送端索引
	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVIF, &on, sizeof(on)) < 0)
		err_ret("setsockopt of IP_RECVIF");
#endif
	bzero(&in_zero, sizeof(struct in_addr));	/* all 0 IPv4 address 初始化一个0.0.0.0的IP4格式地址 */

	for ( ; ; ) {
		len = clilen;
		flags = 0;
		//从sockfd接收一个UDP数据报,存放在缓冲区mesg中,标志存放在flags
		//从地址pcliaddr中接收
		//把地址和索引存放在结构体pktinfo中
		n = Recvfrom_flags(sockfd, mesg, MAXLINE, &flags,
						   pcliaddr, &len, &pktinfo);
		//打印信息,从xx地址接收了xx字节的数据
		//Sock_ntop函数转换二进制地址格式为表达式格式
		printf("%zd-byte datagram from %s", n, Sock_ntop(pcliaddr, len));
		//如果pktinfo结构体中的地址不为0.0.0.0,那么打印出来
		if (memcmp(&pktinfo.ipi_addr, &in_zero, sizeof(in_zero)) != 0)
			printf(", to %s", Inet_ntop(AF_INET, &pktinfo.ipi_addr,
										str, sizeof(str)));
		//如果pktinfo结构体正常返回索引号,打印出索引号
		if (pktinfo.ipi_ifindex > 0)
			printf(", recv i/f = %s",
				   If_indextoname(pktinfo.ipi_ifindex, ifname));
#ifdef	MSG_TRUNC //消息是否被截断
		if (flags & MSG_TRUNC)	printf(" (datagram truncated)");
#endif
#ifdef	MSG_CTRUNC //辅助数据是否被截断
		if (flags & MSG_CTRUNC)	printf(" (control info truncated)");
#endif
#ifdef	MSG_BCAST //是否是广播
		if (flags & MSG_BCAST)	printf(" (broadcast)");
#endif
#ifdef	MSG_MCAST //是否是多播
		if (flags & MSG_MCAST)	printf(" (multicast)");
#endif
		printf("\n");

		Sendto(sockfd, mesg, n, 0, pcliaddr, len); //把接收到的数据回射给客户端
	}
}
