/* include readable_v61 */
#include	"icmpd.h"
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include	<netinet/ip_icmp.h>
#include	<netinet/udp.h>

#ifdef	IPV6
#include	<netinet/ip6.h>
#include	<netinet/icmp6.h>
#endif

int
readable_v6(void)
{
#ifdef	IPV6
	int					i, hlen2, icmp6len, sport;
	char				buf[MAXLINE];
	char				srcstr[INET6_ADDRSTRLEN], dststr[INET6_ADDRSTRLEN];
	ssize_t				n;
	socklen_t			len;
	struct ip6_hdr		*hip6;
	struct icmp6_hdr	*icmp6;
	struct udphdr		*udp;
	struct sockaddr_in6	from, dest;
	struct icmpd_err	icmpd_err;

	len = sizeof(from); //计算IP4地址族的大小(用来供recvfrome使用)
	//fd6全局变量中保存的是ICMP6协议的原始套接字,凡是内核收到ICMP6协议的数据报,都会转发给这个套接字一份
	//recvfrom从这个套接字读取数据报,存放在缓存buf中,并填写对端的IP4地址到from中
	n = Recvfrom(fd6, buf, MAXLINE, 0, (SA *) &from, &len);

	printf("%d bytes ICMPv6 from %s:",
		   (int)n, Sock_ntop_host((SA *) &from, len)); //打印接收到的这个ICMP4数据报的信息
		   
	//IP6数据报的原始套接字收到的消息,缓冲区的第一字节开始就是ICMP首部,IP6首部附加在辅助数据中传输

	icmp6 = (struct icmp6_hdr *) buf;		/* start of ICMPv6 header缓冲区的第一字节是icmp6的首部*/
	if ( (icmp6len = n) < 8) //接收到的整个缓冲区的长度n=icmp6首部+icmp数据区(引发错误的数据报的IP6首部+引发错误的数据报的UDP首部)
		//连存放ICMP6首部的空间都不够,出错退出
		err_quit("icmp6len (%d) < 8", icmp6len);
	
	//有能够存储ICMP首部的空间
	printf(" type = %d, code = %d\n", icmp6->icmp6_type, icmp6->icmp6_code); //打印ICMP首部的一些信息
/* end readable_v61 */

/* include readable_v62 */
	//验证ICMP6数据报的类型
	if (icmp6->icmp6_type == ICMP6_DST_UNREACH || //ICMP类型为:目的端口不可达
		icmp6->icmp6_type == ICMP6_PACKET_TOO_BIG || //ICMP类型为:数据报太大
		icmp6->icmp6_type == ICMP6_TIME_EXCEEDED) { //ICMP类型为:TTL值用尽也没有达到目的地
		
		//icmp6首部(8个字节)+icmp数据区(引发错误的数据报的IP6首部+引发错误的数据报的UDP首部)(8个字节)
		//ICMP数据区只取8个字节是因为我们只需要"引发错误的数据报的IP6首部中前8个字节中保存的源地址和目的地址"
		if (icmp6len < 8 + 8) //查看存储"引发错误的数据报的IP6首部中前8个字节中保存的源地址和目的地址"的空间是否足够
			//空间不足,出错退出
			err_quit("icmp6len (%d) < 8 + 8", icmp6len);
		
		//代码走到这里说明"引发错误的IP6首部之中存放源地址和目的地址的数据区完好"
		//缓冲区的起始字节跨越ICMP首部(8个字节)后,指向的是ICMP数据区的起始字节(也就是引发错误的IP6首部的起始字节)
		hip6 = (struct ip6_hdr *) (buf + 8); //hip指针指向"引发错误的IP6首部"
		hlen2 = sizeof(struct ip6_hdr); //计算一下IP6首部的长度,存入hlen2变量
		printf("\tsrcip = %s, dstip = %s, next hdr = %d\n",
			   Inet_ntop(AF_INET6, &hip6->ip6_src, srcstr, sizeof(srcstr)),
			   Inet_ntop(AF_INET6, &hip6->ip6_dst, dststr, sizeof(dststr)),
			   hip6->ip6_nxt); //打印引发错误的数据报的IP6首部得源地址和目的地址信息
 		if (hip6->ip6_nxt == IPPROTO_UDP) { //查看"引发错误的数据报的IP6首部"中的协议字段,是否是UDP协议
			//"引发错误的数据报的IP6首部"中的协议字段确实是UDP协议
			
			//缓冲区的起始字节跨越ICMP首部(8个字节)后,指向的是ICMP数据区的起始字节(也就是引发错误的IP6首部的起始字节)
			//IP6首部的起始字节跨越整个IP6首部的长度后,就是IP6首部后跟的数据区(UDP首部的起始字节)
			udp = (struct udphdr *) (buf + 8 + hlen2);  //此时udp指针指向UDP首部的首地址
			sport = udp->uh_sport; //取出UDP首部中的源端口号

				/* 4find client's Unix domain socket, send headers */
			//循环遍历client数组,maxi为当前client结构体数组中被使用元素最多的时候,下标所达到的最大值
			for (i = 0; i <= maxi; i++) {
				//根据条件进行查找
				if (client[i].connfd >= 0 && //已经被使用的client结构体数组元素
					client[i].family == AF_INET6 && //这个被使用的client结构体数组元素的地址族是否是IP6地址族
					client[i].lport == sport) { //这个被使用的client结构体数组元素的源端口,和接收缓冲区中提取出来的源端口是否一致
					
					//进入此分支,代表已经找到了这个ICMP错误所对应的本机的客户进程
					
					//构造一个IP6地址
					bzero(&dest, sizeof(dest)); //把IP6地址清空
					dest.sin6_family = AF_INET6; //把这个IP6地址的地址族设置为IP6
#ifdef	HAVE_SOCKADDR_SA_LEN
					dest.sin6_len = sizeof(dest); //填入这个IP6地址的长度
#endif
					memcpy(&dest.sin6_addr, &hip6->ip6_dst,
						   sizeof(struct in6_addr)); //引发错误的数据报的IP6首部中的目的地址,填入到这个待构造的IP6地址中
					dest.sin6_port = udp->uh_dport; //引发错误的数据报的udp首部中的端口号,填入到这个待构造的IP4地址中
													//(地址构造完毕,这个构造完毕的地址,会填入icmpd_err结构体中)
													
					//开始构造icmpd_err结构体

					icmpd_err.icmpd_type = icmp6->icmp6_type; //从ICMP首部中提取ICMP数据报类型,填入icmpd_err
					icmpd_err.icmpd_code = icmp6->icmp6_code; //从ICMP首部中提取ICMP代码,填入icmpd_err
					icmpd_err.icmpd_len = sizeof(struct sockaddr_in6); //icmpd_err中的长度成员是IP6地址的长度(这个长度成员指出了紧跟在它后面的地址长度)
					memcpy(&icmpd_err.icmpd_dest, &dest, sizeof(dest)); //把刚才构造好的地址结构体填入icmpd_err(紧跟在长度成员之后)

						/* 4convert type & code to reasonable errno value */
					icmpd_err.icmpd_errno = EHOSTUNREACH;	/* default 默认设置icmpd_errno的值为EHOSTUNREACH*/
					if (icmp6->icmp6_type == ICMP6_DST_UNREACH && //ICMP数据报的类型是目的端口不可达
						icmp6->icmp6_code == ICMP6_DST_UNREACH_NOPORT) //ICMP数据报的错误代码为:目的端口不可达
						icmpd_err.icmpd_errno = ECONNREFUSED; //设置icmpd_errno的值为ECONNREFUSED
					if (icmp6->icmp6_type == ICMP6_PACKET_TOO_BIG) //ICMP数据报的错误代码为:需分片,但DF位已设置
							icmpd_err.icmpd_errno = EMSGSIZE; //设置icmpd_errno的值为EMSGSIZE
							
					Write(client[i].connfd, &icmpd_err, sizeof(icmpd_err)); //把这个构造好的icmpd_err结构体发送给客户进程
				}
			}
		}
	}
	return(--nready); //待处理的描述符个数-1,函数返回
#endif
}
/* end readable_v62 */
