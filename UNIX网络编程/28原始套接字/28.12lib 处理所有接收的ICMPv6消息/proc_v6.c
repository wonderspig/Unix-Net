#include	"ping.h"

void
proc_v6(char *ptr, ssize_t len, struct msghdr *msg, struct timeval* tvrecv)
{
#ifdef	IPV6
	double				rtt; //rtt网络延时
	struct icmp6_hdr	*icmp6; //ICMP6首部结构体
	struct timeval		*tvsend; //数据发送时的时间点
	struct cmsghdr		*cmsg; //存放附加数据的结构体(IP6首部存放在应答的附加数据中)
	int					hlim; //从附加数据中读出IP6的跳限存放在这个变量中

	icmp6 = (struct icmp6_hdr *) ptr; //接受缓冲区的收地址,存放的就是ICMP6的首部
	if (len < 8) //如果接受缓冲区的总长度小于8,那么连ICMP首部的长度都存放不下,函数直接返回
		return;				/* malformed packet */

	if (icmp6->icmp6_type == ICMP6_ECHO_REPLY) { //通过读取ICMP首部结构体,判断这是不是一个ICMP回射应答
		if (icmp6->icmp6_id != pid) //如果时ICMP回射应答,就检查其中的标识符字段,是否为本进程IP
			return;			/* not a response to our ECHO_REQUEST 不是本进程IP,函数直接返回*/
		if (len < 16) //缓冲区的长度小于16(接收缓冲区中的存放的是ICMP首部(8字节)+ICMP数据(8字节))
			return;			/* not enough data to use 长度小于16,函数直接返回*/

		tvsend = (struct timeval *) (icmp6 + 1); //ICMP6的指针+1,代表指针现在跳过ICMP结构体,指向ICMP结构体之后的位置(ICMP数据的起始位置)
		tv_sub(tvrecv, tvsend); //把数据接收的时间点-数据发送的时间点的值,存入tvrecv
		rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0; //把差值结果的时间点转换为秒

		hlim = -1; //IP6跳限初始化为-1
		//用循环读取附加数据,附加数据中存放的是IP6首部的信息(我们要取的是其中的跳限值,如果有的话)
		for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg)) {
			if (cmsg->cmsg_level == IPPROTO_IPV6 &&
				cmsg->cmsg_type == IPV6_HOPLIMIT) { //筛选出辅助数据的保存跳限的辅助数据
				hlim = *(u_int32_t *)CMSG_DATA(cmsg); //把辅助数据中的跳限值拿出,存放在变量hlim中
				break; //跳出循环(我们只要跳限数据)
			}
		}
		printf("%d bytes from %s: seq=%u, hlim=",
				(int)len, Sock_ntop_host(pr->sarecv, pr->salen),
				icmp6->icmp6_seq); //打印收到数据的长度,地址,序列号
		if (hlim == -1)
			printf("???");	/* ancillary data missing */
		else
			printf("%d", hlim); //如果有跳限就打印跳限
		printf(", rtt=%.3f ms\n", rtt); //打印rtt网络延时的值
	} else if (verbose) { //添加了-v参数,就打印更详细的信息
		printf("  %d bytes from %s: type = %d, code = %d\n",
				(int)len, Sock_ntop_host(pr->sarecv, pr->salen),
				icmp6->icmp6_type, icmp6->icmp6_code);
	}
#endif	/* IPV6 */
}
