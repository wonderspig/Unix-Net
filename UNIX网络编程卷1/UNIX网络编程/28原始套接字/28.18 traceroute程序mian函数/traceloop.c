#include	"trace.h"

void
traceloop(void)
{
	int					seq, code, done; //序列号  ICMP错误代码  是否完成的标志量done
	double				rtt; //网络延时
	struct rec			*rec; //发送的UDP数据报的数据部分结构体
	struct timeval		tvrecv; //接收消息的时间点
	//创建接收套接字,根据addrinfo查找到的对端地址的地址族,ICMP协议,创建原始套接字
	recvfd = Socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
	setuid(getuid());		/* don't need special permissions anymore 把权限设置回有效用户ID*/

#ifdef	IPV6 //如果系统支持IP6协议
	if (pr->sasend->sa_family == AF_INET6 && verbose == 0) { //判断addrinfo查找到的地址的地址族是否为IP6地址族
		struct icmp6_filter myfilt; //定义一个IP6的过滤器
		ICMP6_FILTER_SETBLOCKALL(&myfilt); //把过滤器设置为阻塞全部消息
		ICMP6_FILTER_SETPASS(ICMP6_TIME_EXCEEDED, &myfilt); //此ICMP分组到达表示TTL跳限值用尽,也没有到达目的地
		ICMP6_FILTER_SETPASS(ICMP6_DST_UNREACH, &myfilt); //允许目的端口不可达的ICMP分组到达(已经到达了目的地)
		setsockopt(recvfd, IPPROTO_IPV6, ICMP6_FILTER,
					&myfilt, sizeof(myfilt)); //把设置好的过滤器设置进套接字选项
	}
#endif

	sendfd = Socket(pr->sasend->sa_family, SOCK_DGRAM, 0); //创建发送套接字,根据addrinfo查找到的对端地址的地址族,UDP协议
	
	//开始为套接字绑定源端口号(开始设置待绑定的通用地址结构体)
	pr->sabind->sa_family = pr->sasend->sa_family; //待绑定的结构体的地址族是addrinfo查找到的对端地址的地址族
	sport = (getpid() & 0xffff) | 0x8000;	/* our source UDP port # 随机生成待绑定的源端口*/
	sock_set_port(pr->sabind, pr->salen, htons(sport)); //把生成好的源端口填入到待绑定的结构体中
	Bind(sendfd, pr->sabind, pr->salen); //地址结构体设置完毕,里面已经包含了源端口号,把这个地址结构体绑定到套接字上

	sig_alrm(SIGALRM); //手动调用信号处理函数

	seq = 0; //序列号初始化为0
	done = 0; //是否完成的标记初始化为0(未完成状态)
	//开始用循环发送探测分组(TTL值从1开始累加,每一个TTL值发送nprobes个探测分组,nprobes默认为3)
	for (ttl = 1; ttl <= max_ttl && done == 0; ttl++) {
		Setsockopt(sendfd, pr->ttllevel, pr->ttloptname, &ttl, sizeof(int)); //用套接字选项来设置TTL值
		bzero(pr->salast, pr->salen); //pr->salast成员是一个通用地址结构体,存放的是最后一次探测对端主机的地址,初始化为0

		printf("%2d ", ttl); //打印TTL值
		fflush(stdout);

		for (probe = 0; probe < nprobes; probe++) { //开始发送探测分组,发送nprobes次
			rec = (struct rec *) sendbuf; //发送缓冲区为rec结构体
			rec->rec_seq = ++seq; //rec的序列号成员初始化为0
			rec->rec_ttl = ttl; //rec的TTL值初始化为当前的TTL值
			Gettimeofday(&rec->rec_tv, NULL); //获得当前时间,此为发送时间

			sock_set_port(pr->sasend, pr->salen, htons(dport + seq)); //为addrinfo找到的对端地址填入端口号(期待这个端口号对方主机没有使用)
			//向对端发送数据(如果出错,这个数据包会附在ICMP的数据区反送回来)
			Sendto(sendfd, sendbuf, datalen, 0, pr->sasend, pr->salen); 

			if ( (code = (*pr->recv)(seq, &tvrecv)) == -3) //调用ICMP4或ICMP6的接收函数,处理反送回来的ICMP数据包
				//返回值是-3代表超时没有应答
				printf(" *");		/* timeout, no reply */
			else {
				char	str[NI_MAXHOST]; //用来存放接收数据报时的对端地址的主机名
				
				//经过ICMP4或ICMP6的接收函接收数据之后
				//这两个函数会设置全局变量pr->sarecv用来保存接收数据报时对端的地址
				//把每一次设置好的pr->sarecv变量和pr->salast作比较,如果不相同,先打印则pr->sarecv的内容,再把pr->sarecv变量拷贝到pr->salast变量
				//若此做的目的是:如果对于某指定TTL,IP地址发生了变化,就可以根据打印内容跟踪这个IP的变化
				if (sock_cmp_addr(pr->sarecv, pr->salast, pr->salen) != 0) {
					if (getnameinfo(pr->sarecv, pr->salen, str, sizeof(str),
									NULL, 0, 0) == 0) //解析出"接收数据报时对端的地址"中的主机名,保存在str中
						//如果能够解析出"接收数据报时对端的地址"的主机名
						printf(" %s (%s)", str,
								Sock_ntop_host(pr->sarecv, pr->salen)); //打印对端主机名,和主机地址
					else
						//如果不能够解析出"接收数据报时对端的地址"的主机名
						printf(" %s",
								Sock_ntop_host(pr->sarecv, pr->salen)); //打印对端主机地址
					//拷贝"接收端数据报时的对端主机地址"到"最后一次发送数据报时的对端地址"
					memcpy(pr->salast, pr->sarecv, pr->salen);
				}
				tv_sub(&tvrecv, &rec->rec_tv); //接收数据报的时间点-发送数据报的时间点,并存入前者
				rtt = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0; //用时间点之差计算式RRT
				printf("  %.3f ms", rtt); //打印RRT信息
				
				//返回值不是-3的情况进行处理
				//返回值是-2说明TTL用尽,还没有到达对端
				//返回值是-1代表已经到达目的地
				
				if (code == -1)		/* port unreachable; at destination 返回值是-1代表目标主机虽然存在,但是端口不可达(探测分组已经到达目的地)*/
					done++; //递增变量done退出循环
				else if (code >= 0) //code大于0,,表示一个常规ICMP数据错误
					printf(" (ICMP %s)", (*pr->icmpcode)(code)); //打印ICMP错误信息
			}
			fflush(stdout); //刷新输出缓冲区
		}
		printf("\n");
	}
}
