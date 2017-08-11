/* include readable_v41 */
#include	"icmpd.h"
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include	<netinet/ip_icmp.h>
#include	<netinet/udp.h>

int
readable_v4(void)
{
	int					i, hlen1, hlen2, icmplen, sport;
	char				buf[MAXLINE];
	char				srcstr[INET_ADDRSTRLEN], dststr[INET_ADDRSTRLEN];
	ssize_t				n;
	socklen_t			len;
	struct ip			*ip, *hip;
	struct icmp			*icmp;
	struct udphdr		*udp;
	struct sockaddr_in	from, dest;
	struct icmpd_err	icmpd_err;

	len = sizeof(from); //计算IP4地址族的大小(用来供recvfrome使用)
	//fd4全局变量中保存的是ICMP4协议的原始套接字,凡是内核收到ICMP4协议的数据报,都会转发给这个套接字一份
	//recvfrom从这个套接字读取数据报,存放在缓存buf中,并填写对端的IP4地址到from中
	n = Recvfrom(fd4, buf, MAXLINE, 0, (SA *) &from, &len); //

	printf("%d bytes ICMPv4 from %s:",
		   (int)n, Sock_ntop_host((SA *) &from, len)); //打印接收到的这个ICMP4数据报的信息

	ip = (struct ip *) buf;		/* start of IP header 接收缓冲区的第一个字节开始,是这个IP数据报的首部*/
	hlen1 = ip->ip_hl << 2;		/* length of IP header IP首部+IP选项的长度 */

	icmp = (struct icmp *) (buf + hlen1);	/* start of ICMP header 跨越过IP首部和IP选项的长度之后,是ICMP首部的第一个字节*/
	if ( (icmplen = n - hlen1) < 8) //整个数据报的长度-IP首部-IP选项的长度=ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)
		//ICMP首部+ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)的值小于8,说明连ICMP首部的空间都不足
		err_quit("icmplen (%d) < 8", icmplen); //出错退出
	
	//有能够存储ICMP首部的空间
	printf(" type = %d, code = %d\n", icmp->icmp_type, icmp->icmp_code); //打印ICMP首部的一些信息
/* end readable_v41 */

/* include readable_v42 */
	//判断ICMP类型
	if (icmp->icmp_type == ICMP_UNREACH || //目的地端口不可达
		icmp->icmp_type == ICMP_TIMXCEED || //TTL值用尽也没有达到目的地主机
		icmp->icmp_type == ICMP_SOURCEQUENCH) { //源熄灭
		//icmplen为ICMP首部+ICMP数据长度(错误数据报的IP首部+错误数据报的IP选项+UDP首部)
		if (icmplen < 8 + 20 + 8) //判断"ICMP首部+ICMP数据区长度(错误数据报的IP首部+错误数据报的IP选项+UDP首部)"的长度是否足够
			err_quit("icmplen (%d) < 8 + 20 + 8", icmplen); //长度不够,打印相应的出错消息

		//代码走到这里，说明ICMP首部和ICMP数据区(错误数据报的IP首部+错误数据报的IP选项+UDP首部)长度足够,数据完整
		//接收缓冲区的首地址,跨越IP首部和IP选项的长度之后,达到ICMP首部的首地址
		//ICMP首部的首地址+8之后跨越了ICMP首部(ICMP首部为8个字节)
		hip = (struct ip *) (buf + hlen1 + 8); //hip指针指向ICMP数据区的首地址(这里存放的是引发错误的数据报的IP首部的首地址)
		hlen2 = hip->ip_hl << 2; //hlen2保存了"发错误的数据报的IP首部+发错误的数据报的IP选项"的长度
		printf("\tsrcip = %s, dstip = %s, proto = %d\n",
			   Inet_ntop(AF_INET, &hip->ip_src, srcstr, sizeof(srcstr)),
			   Inet_ntop(AF_INET, &hip->ip_dst, dststr, sizeof(dststr)),
			   hip->ip_p); //打印引发错误的数据报的IP首部中的源地址和目的地址信息
 		if (hip->ip_p == IPPROTO_UDP) { //引发错误的数据报的ip首部中的协议是否是UDP协议
			//引发错误的数据报的ip首部后跟的是UDP首部
			//接收缓冲区的首地址,跨越IP首部和IP选项的长度之后,达到ICMP首部的首地址
			//ICMP首部的首地址+8之后跨越了ICMP首部(ICMP首部为8个字节),达到了ICMP数据区的首地址(这里存放的是引发错误的数据报的IP首部的首地址)
			//跨越了"发错误的数据报的IP首部+发错误的数据报的IP选项"的长度之后,指向了引发错误的数据报的IP首部之后的数据区(数据区中存放的是UDP首部)
			udp = (struct udphdr *) (buf + hlen1 + 8 + hlen2); //此时udp指针指向UDP首部的首地址
			sport = udp->uh_sport; //取出UDP首部中的源端口号

				/* 4find client's Unix domain socket, send headers */
			//循环遍历client数组,maxi为当前client结构体数组中被使用元素最多的时候,下标所达到的最大值
			for (i = 0; i <= maxi; i++) {
				//根据条件进行查找
				if (client[i].connfd >= 0 && //已经被使用的client结构体数组元素
					client[i].family == AF_INET && //这个被使用的client结构体数组元素的地址族是否是IP4地址族
					client[i].lport == sport) { //这个被使用的client结构体数组元素的源端口,和接收缓冲区中提取出来的源端口是否一致
					
					//进入此分支,代表已经找到了这个ICMP错误所对应的本机的客户进程
					//构造一个IP4地址
					bzero(&dest, sizeof(dest)); //把IP4地址清空
					dest.sin_family = AF_INET; //把这个IP4地址的地址族设置为IP4
#ifdef	HAVE_SOCKADDR_SA_LEN
					dest.sin_len = sizeof(dest); //填入这个IP4地址的长度
#endif
					memcpy(&dest.sin_addr, &hip->ip_dst,
						   sizeof(struct in_addr)); //引发错误的数据报的IP首部中的目的地址,填入到这个待构造的IP4地址中
					dest.sin_port = udp->uh_dport; //引发错误的数据报的udp首部中的端口号,填入到这个待构造的IP4地址中
													//(地址构造完毕,这个构造完毕的地址,会填入icmpd_err结构体中)

													
					//开始构造icmpd_err结构体
					
					icmpd_err.icmpd_type = icmp->icmp_type; //从ICMP首部中提取ICMP数据报类型,填入icmpd_err
					icmpd_err.icmpd_code = icmp->icmp_code; //从ICMP首部中提取ICMP代码,填入icmpd_err
					icmpd_err.icmpd_len = sizeof(struct sockaddr_in); //icmpd_err中的长度成员是IP4地址的长度(这个长度成员指出了紧跟在它后面的地址长度)
					memcpy(&icmpd_err.icmpd_dest, &dest, sizeof(dest)); //把刚才构造好的地址结构体填入icmpd_err(紧跟在长度成员之后)

						/* 4convert type & code to reasonable errno value */
					icmpd_err.icmpd_errno = EHOSTUNREACH;	/* default 默认设置icmpd_errno的值为EHOSTUNREACH*/
					if (icmp->icmp_type == ICMP_UNREACH) { //ICMP数据报的类型是目的端口不可达
						if (icmp->icmp_code == ICMP_UNREACH_PORT) //ICMP数据报的错误代码为:目的端口不可达
							icmpd_err.icmpd_errno = ECONNREFUSED; //设置icmpd_errno的值为ECONNREFUSED
						else if (icmp->icmp_code == ICMP_UNREACH_NEEDFRAG) //ICMP数据报的错误代码为:需分片,但DF位已设置
							icmpd_err.icmpd_errno = EMSGSIZE; //设置icmpd_errno的值为EMSGSIZE
					}
					Write(client[i].connfd, &icmpd_err, sizeof(icmpd_err)); //把这个构造好的icmpd_err结构体发送给客户进程
				}
			}
		}
	}
	return(--nready); //待处理的描述符个数-1,函数返回
}
/* end readable_v42 */
