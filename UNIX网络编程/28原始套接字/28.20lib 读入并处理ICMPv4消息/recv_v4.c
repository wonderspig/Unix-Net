#include	"trace.h"

extern int gotalarm;

/*
 * Return: -3 on timeout
 *		   -2 on ICMP time exceeded in transit (caller keeps going)
 *		   -1 on ICMP port unreachable (caller is done)
 *		 >= 0 return value is some other ICMP unreachable code
 */

int
recv_v4(int seq, struct timeval *tv)
{
	//hlen1: 返回数据报的IP首部长度
	//hlen2: 引起错误的IP数据报会附加在ICMP首部后的数据区返回,这是ICMP首部后的数据区中引起错误的数据报的IP首部(也就是发送的那个UDP数据报)
	//icmplen: ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)的长度
	//ret: 函数的返回值
	int				hlen1, hlen2, icmplen, ret; 
	socklen_t		len; //套接字地址结构的的长度
	ssize_t			n; //整个数据报的长度:IP4首部+IP4选项+ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)
	struct ip		*ip, *hip; //ip:IP首部结构体  hip:错误数据报的IP首部结构体
	struct icmp		*icmp; //ICMP首部
	struct udphdr	*udp; //UDP首部

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

		ip = (struct ip *) recvbuf;	/* start of IP header 接收缓冲区的第一个字节开始,是这个IP数据报的首部*/
		hlen1 = ip->ip_hl << 2;		/* length of IP header IP首部+IP选项的长度*/
	
		icmp = (struct icmp *) (recvbuf + hlen1); /* start of ICMP header 跨越过IP首部和IP选项的长度之后,是ICMP首部的第一个字节*/
		if ( (icmplen = n - hlen1) < 8) //整个数据报的长度-IP首部-IP选项的长度=ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)
			//ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)的值小于8,说明连ICMP首部的空间都不足
			continue;				/* not enough to look at ICMP header 继续循环*/
		
		//有足够存放ICMP首部的空间
		if (icmp->icmp_type == ICMP_TIMXCEED && 
			icmp->icmp_code == ICMP_TIMXCEED_INTRANS) { //icmp数据报类型是:TTL跳限值用尽也没有到达目的地的错误
			//ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)的值小于"8(ICMP首部长度)+IP首部长度"
			if (icmplen < 8 + sizeof(struct ip)) 
				//说明没有存放IP首部的足够空间(错误数据报的IP首部)
				continue;			/* not enough data to look at inner IP 继续循环*/
			
			//有足够存放IP首部的空间
			//hip指针指向IP首部(错误数据报的IP首部)
			//接收缓冲区开始的第一个字节,跨越IP首部和IP选项之后是ICMP首部的第一个字节,跨越ICMP首部(8个字节)之后就是ICMP数据区
			//icmp数据区包括:错误数据报的IP首部+错误数据报的IP选项+UDP首部
			hip = (struct ip *) (recvbuf + hlen1 + 8); 
			hlen2 = hip->ip_hl << 2; //存取引起错误的数据报的IP首部的指针
			if (icmplen < 8 + hlen2 + 4) //验证是否有足够的空间容纳UDP首部(+4是因为我们只需要其中的端口号,后面的四个字节UDP首部数据无关紧要)
				//没有足够的空间容纳UDP首部
				continue;			/* not enough data to look at UDP ports 继续循环*/

			udp = (struct udphdr *) (recvbuf + hlen1 + 8 + hlen2); //提取UDP数据报的指针
 			if (hip->ip_p == IPPROTO_UDP && //验证UDP协议
				udp->uh_sport == htons(sport) && //验证UDP源端口是否是本进程发送时的源端口
				udp->uh_dport == htons(dport + seq)) { //验证UDP的目的端口,是否是本进程发送时的目的端口
				//以上判断都成立就代表确实是中间某个路由器对我们探测分组的一个回应
				ret = -2;		/* we hit an intermediate router 函数返回-2,且跳出循环*/
				break;
			}

		} else if (icmp->icmp_type == ICMP_UNREACH) { //ICMP消息是目的端口不可达
			//ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)的值小于"8(ICMP首部长度)+IP首部长度"
			if (icmplen < 8 + sizeof(struct ip)) //
				//说明没有存放IP首部的足够空间(错误数据报的IP首部)
				continue;			/* not enough data to look at inner IP 继续循环*/
			
			//有足够存放IP首部的空间
			//hip指针指向IP首部(错误数据报的IP首部)
			//接收缓冲区开始的第一个字节,跨越IP首部和IP选项之后是ICMP首部的第一个字节,跨越ICMP首部(8个字节)之后就是ICMP数据区
			//icmp数据区包括:错误数据报的IP首部+错误数据报的IP选项+UDP首部
			hip = (struct ip *) (recvbuf + hlen1 + 8);
			hlen2 = hip->ip_hl << 2; //存取引起错误的数据报的IP首部的指针
			if (icmplen < 8 + hlen2 + 4) //验证是否有足够的空间容纳UDP首部(+4是因为我们只需要其中的端口号,后面的四个字节UDP首部无关紧要)
				continue;			/* not enough data to look at UDP ports 继续循环*/

			udp = (struct udphdr *) (recvbuf + hlen1 + 8 + hlen2); //提取UDP数据报的指针
 			if (hip->ip_p == IPPROTO_UDP && //验证UDP协议
				udp->uh_sport == htons(sport) && //验证UDP源端口是否是本进程发送时的源端口
				udp->uh_dport == htons(dport + seq)) { //验证UDP的目的端口,是否是本进程发送时的目的端口
				if (icmp->icmp_code == ICMP_UNREACH_PORT) //验证ICMP的错误代码是否是目的端口不可达
					//是目的端口不可达的错误,函数就返回-1
					ret = -1;	/* have reached destination */
				else
					//不是目的端口不可达的错误,就把具体的ICMP错误代码返回
					ret = icmp->icmp_code;	/* 0, 1, 2, ... */
				break; 
			}
		}
		if (verbose) { //命令行参数有-v选项,就打印更详细的信息
			printf(" (from %s: type = %d, code = %d)\n",
					Sock_ntop_host(pr->sarecv, pr->salen),
					icmp->icmp_type, icmp->icmp_code);
		}
		/* Some other ICMP error, recvfrom() again */
	}
	alarm(0);					/* don't leave alarm running 定时器设置为0*/
	Gettimeofday(tv, NULL);		/* get time of packet arrival 填写接收数据报时候的时间点*/
	return(ret); //返回函数的结果
}
