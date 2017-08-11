#include "ping.h"

void
init_v6()
{
#ifdef IPV6
	int on = 1; //用来打开套接字选项

	if (verbose == 0) { //如果命令行参数没有指定-v
		/* install a filter that only passes ICMP6_ECHO_REPLY unless verbose */
		struct icmp6_filter myfilt; //定义ICMP6的过滤器结构体
		ICMP6_FILTER_SETBLOCKALL(&myfilt); //过滤器结构体设置为阻塞所有消息
		ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &myfilt); //过滤器结构体设置为只接收回射应答类型的消息(ICMP6协议字段不包含IP首部,IP首部通过附加数据返回)
		setsockopt(sockfd, IPPROTO_IPV6, ICMP6_FILTER, &myfilt, sizeof(myfilt)); //把设置好的过滤器结构体,设置进套接字选项
		/* ignore error return; the filter is an optimization */
	}

	/* ignore error returned below; we just won't receive the hop limit */
#ifdef IPV6_RECVHOPLIMIT //根据版本的不同,设置IP6接收跳限的套接字选项
	/* RFC 3542 */
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on));
#else
	/* RFC 2292 */
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_HOPLIMIT, &on, sizeof(on));
#endif
#endif
}
