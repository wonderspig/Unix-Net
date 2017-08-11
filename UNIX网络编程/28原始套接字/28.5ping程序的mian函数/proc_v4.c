#include	"ping.h"

//使用IP4的协议来处理缓冲区中的内容
void
proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv)
{
	int				hlen1, icmplen; //hlen1:IP4首部＋IP4选项的长度  icmplen:icmp首部+ICMP数据的长度
	double			rtt; //网络延时
	struct ip		*ip; //IP4首部的结构体地址
	struct icmp		*icmp; //ICMP4首部的结构体地址
	struct timeval	*tvsend; //发送数据时的时间点(放在icmp4数据区一起发送,会随icmp4数据区返回)

	ip = (struct ip *) ptr;		/* start of IP header 接受缓冲区的首字节位置,存放的时IP4首部*/
	hlen1 = ip->ip_hl << 2;		/* length of IP header IP4首部+IP4选项*/
	if (ip->ip_p != IPPROTO_ICMP) //如果IP首部中的协议项不是ICMP4
		return;				/* not ICMP 直接返回*/

	//接收缓冲区的开始处,向后移动hlen1,正好指向ICMP首部的起始位置
	icmp = (struct icmp *) (ptr + hlen1);	/* start of ICMP header */
	if ( (icmplen = len - hlen1) < 8) //整个缓冲区长度-IP4首部的长度-IP4选项的长度=ICMP首部+ICMP数据的长度
		return;				/* malformed packet 此长度小于8,说明连存放ICMP首部的空间都不够,直接返回*/

	//有了存放ICMP首部的空间,说明ICMP首部有了
	if (icmp->icmp_type == ICMP_ECHOREPLY) { //访问ICMP首部的空间icmp_type成员,此ICMP消息类型,是一个ICMP应答
		if (icmp->icmp_id != pid) //查看ICMP首部的标识字段,如果不是本进程的PID,则直接返回
			return;			/* not a response to our ECHO_REQUEST */
		if (icmplen < 16) //ICMP首部+ICMP数据的长度的值小于16,说明没有足够的空间存放ICMP数据,函数返回
			return;			/* not enough data to use */

		//代码到达这里说明,ICMP数据区完好
		tvsend = (struct timeval *) icmp->icmp_data; //取出ICMP数据区,数据区存放的是发送时的时间点
		tv_sub(tvrecv, tvsend); //把数据接收的时间点-数据发送的时间点的值,存入tvrecv
		rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0; //把差值结果的时间点转换为秒

		printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n", //打印trr时间
				icmplen, Sock_ntop_host(pr->sarecv, pr->salen),
				icmp->icmp_seq, ip->ip_ttl, rtt);

	} else if (verbose) { //如果命令行指定的-v参数,更详细的打印rtt时间
		printf("  %d bytes from %s: type = %d, code = %d\n",
				icmplen, Sock_ntop_host(pr->sarecv, pr->salen),
				icmp->icmp_type, icmp->icmp_code);
	}
}
