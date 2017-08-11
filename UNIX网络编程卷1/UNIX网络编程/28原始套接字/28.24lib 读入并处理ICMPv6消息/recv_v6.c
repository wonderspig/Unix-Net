#include	"trace.h"

extern int gotalarm;

/*
 * Return: -3 on timeout
 *		   -2 on ICMP time exceeded in transit (caller keeps going)
 *		   -1 on ICMP port unreachable (caller is done)
 *		 >= 0 return value is some other ICMP unreachable code
 */

int
recv_v6(int seq, struct timeval *tv)
{
#ifdef	IPV6

	//ICMP6数据报将不包含本数据报的IP首部,接收到数据报的起始位置,直接就是ICMP首部
	
	//hlen2:引起错误的IP数据报会附加在ICMP首部后的数据区返回,这是ICMP首部后的数据区中引起错误的数据报的IP首部(也就是发送的那个UDP数据报)
	//icmp6len:整个ICMP6数据报的总长度
	//ret:函数的返回值
	int					hlen2, icmp6len, ret;
	ssize_t				n; //recvfrom读取到的数据的总长度
	socklen_t			len; //套接字地址结构体的长度
	struct ip6_hdr		*hip6; //IP6首部结构体
	struct icmp6_hdr	*icmp6; //ICMP6首部结构体
	struct udphdr		*udp; //UDP首部结构体

	gotalarm = 0; //标志量:ararm信号是否出现过
	alarm(3); //3秒后将会收到alarm信号
	for ( ; ; ) {
		if (gotalarm) //alarm信号发生过了(对端超时没有响应,recvform超过3秒还没有接收到数据,看做对端没有响应)
			return(-3);		/* alarm expired 函数返回-3*/
		len = pr->salen; //保存套接字地址结构的长度(因为recvfrom会改变传入的len的大小)
		n = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, pr->sarecv, &len); //把读到的所有数据的总长度,保存在变量n中
		if (n < 0) { //recvfrom出错
			if (errno == EINTR) //错误是被信号打断的
				continue; //继续循环
			else
				err_sys("recvfrom error"); //其他错误表示无法接受
		}

		icmp6 = (struct icmp6_hdr *) recvbuf; /* ICMP header 接收缓冲区的第一个字节开始,是ICMP6的首部*/
		if ( ( icmp6len = n ) < 8) //因为是ICMP6协议,所以读取到的缓冲区的总大小就等于"ICMP6首部+ICMP6数据区(错误数据报的IP6首部+UDP首部)"
			//这个大小小于8就表示连ICMP6首部的空间都不够(直接继续循环)
			continue;				/* not enough to look at ICMP header 继续循环*/
	
		if (icmp6->icmp6_type == ICMP6_TIME_EXCEEDED &&
			icmp6->icmp6_code == ICMP6_TIME_EXCEED_TRANSIT) { //icmp数据报类型是:TTL跳限值用尽也没有到达目的地的错误
			
			//"ICMP6首部+ICMP6数据区(错误数据报的IP6首部+UDP首部)"小于
			//"ICMP6首部(8个字节)+错误数据报的IP6首部+UDP首部(+4是因为我们只需要其中的端口号,后面的四个字节UDP首部数据无关紧要)"
			//如果这个小于成立,就表示没有足够的空间来存放UDP首部中的端口信息
			if (icmp6len < 8 + sizeof(struct ip6_hdr) + 4) 
				//没有足够的空间来存放UDP首部中的端口信息
				continue;			/* not enough data to look at inner header 继续循环*/

			hip6 = (struct ip6_hdr *) (recvbuf + 8); //把"错误数据报的IP6首部"的指针存放在hip6中
			hlen2 = sizeof(struct ip6_hdr); //计算引起错的IP6数据报的首部长度
			//提取UDP数据报的指针
			//接收缓冲区的开始位置,跨越ICMP首部(8个字节)后是引起错误的数据报的IP6首部
			//跨越引起错误的数据报的IP6首部后是UDP首部的开始位置
			udp = (struct udphdr *) (recvbuf + 8 + hlen2);
			if (hip6->ip6_nxt == IPPROTO_UDP && //判断IP6首部中的协议是否是UDP协议
				udp->uh_sport == htons(sport) && //判断UDP首部中的源端口,是否是本进程发送时的源端口
				udp->uh_dport == htons(dport + seq)) //判断UDP首部中的目的端口,是否是本进程发送时的目的端口
				//以上判断都成立就代表确实是中间某个路由器对我们探测分组的一个回应
				ret = -2;		/* we hit an intermediate router 函数返回-2,且跳出循环*/
				break;

		} else if (icmp6->icmp6_type == ICMP6_DST_UNREACH) { //ICMP消息是目的端口不可达
		
			//"ICMP6首部+ICMP6数据区(错误数据报的IP6首部+UDP首部)"小于
			//"ICMP6首部(8个字节)+错误数据报的IP6首部+UDP首部(+4是因为我们只需要其中的端口号,后面的四个字节UDP首部数据无关紧要)"
			//如果这个小于成立,就表示没有足够的空间来存放UDP首部中的端口信息
			if (icmp6len < 8 + sizeof(struct ip6_hdr) + 4)
				//没有足够的空间来存放UDP首部中的端口信息
				continue;			/* not enough data to look at inner header 继续循环*/

			hip6 = (struct ip6_hdr *) (recvbuf + 8); //把"错误数据报的IP6首部"的指针存放在hip6中
			hlen2 = sizeof(struct ip6_hdr); //计算引起错的IP6数据报的首部长度
			//提取UDP数据报的指针
			//接收缓冲区的开始位置,跨越ICMP首部(8个字节)后是引起错误的数据报的IP6首部
			//跨越引起错误的数据报的IP6首部后是UDP首部的开始位置
			udp = (struct udphdr *) (recvbuf + 8 + hlen2); 
			if (hip6->ip6_nxt == IPPROTO_UDP && //判断IP6首部中的协议是否是UDP协议
				udp->uh_sport == htons(sport) && //判断UDP首部中的源端口,是否是本进程发送时的源端口
				udp->uh_dport == htons(dport + seq)) { //判断UDP首部中的目的端口,是否是本进程发送时的目的端口
				if (icmp6->icmp6_code == ICMP6_DST_UNREACH_NOPORT) //如果ICMP6的错误代码是目的端口不可达
					//是目的端口不可达的错误,函数就返回-1
					ret = -1;	/* have reached destination */
				else
					//不是目的端口不可达的错误,就把具体的ICMP错误代码返回
					ret = icmp6->icmp6_code;	/* 0, 1, 2, ... */
				break;
			}
		} else if (verbose) { //命令行参数有-v选项,就打印更详细的信息
			printf(" (from %s: type = %d, code = %d)\n",
					Sock_ntop_host(pr->sarecv, pr->salen),
					icmp6->icmp6_type, icmp6->icmp6_code);
		}
		/* Some other ICMP error, recvfrom() again */
	}
	alarm(0);					/* don't leave alarm running 定时器设置为0*/
	Gettimeofday(tv, NULL);		/* get time of packet arrival 填写接收数据报时候的时间点*/
	return(ret); //返回函数的结果
#endif
}
