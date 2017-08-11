#include	"trace.h"
//初始化proto为ICMP4协议
struct proto	proto_v4 = { icmpcode_v4, recv_v4, NULL, NULL, NULL, NULL, 0,
							 IPPROTO_ICMP, IPPROTO_IP, IP_TTL };
//初始化proto为ICMP6协议
#ifdef	IPV6
struct proto	proto_v6 = { icmpcode_v6, recv_v6, NULL, NULL, NULL, NULL, 0,
							 IPPROTO_ICMPV6, IPPROTO_IPV6, IPV6_UNICAST_HOPS };
#endif

int		datalen = sizeof(struct rec);	/* defaults ICMP的数据部分就是结构体rec的大小*/
int		max_ttl = 30; //最大的跳限上限值
int		nprobes = 3; //每个跳限发多少个探测包
u_short	dport = 32768 + 666; //目标端口

int
main(int argc, char **argv)
{
	int				c; //命令行选项
	struct addrinfo	*ai; //addrinfo所需要的结构体(用来查找合适的对端地址)
	char *h; //主机名

	opterr = 0;		/* don't want getopt() writing to stderr getopt函数出错不输出到标准输出*/
	while ( (c = getopt(argc, argv, "m:v")) != -1) {
		switch (c) {
		case 'm': //如果是-m参数,则取-m参数后具体的属性值(最大ttl跳限值)
			if ( (max_ttl = atoi(optarg)) <= 1)
				err_quit("invalid -m value");
			break;

		case 'v': //-v参数表示更详细的显示信息
			verbose++;
			break;

		case '?': //出错
			err_quit("unrecognized option: %c", c);
		}
	}

	if (optind != argc-1) //不是以-开头的参数,应该是命令行的最后一个参数
		err_quit("usage: traceroute [ -m <maxttl> -v ] <hostname>");
	host = argv[optind]; //最后一个参数是主机名,把它存入host变量中

	pid = getpid(); //保存一下本进程的PID
	Signal(SIGALRM, sig_alrm); //注册信号处理程序

	ai = Host_serv(host, NULL, 0, 0); //把用户传参进来的主机名作为addrinfo的参数,找出合适的地址,存放在ai中

	h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen); //把ai中已经找到的对端地址对应成主机名,保存在h中
	printf("traceroute to %s (%s): %d hops max, %d data bytes\n",
		   ai->ai_canonname ? ai->ai_canonname : h,
		   h, max_ttl, datalen); //打印对端的地址信息

		/* initialize according to protocol */
	if (ai->ai_family == AF_INET) { //判断对端的地址族,是否为IP4地址族
		pr = &proto_v4; //是IP4地址族,则把初始化为IP4的proto_v4结构体附值给pr全局指针
#ifdef	IPV6
	} else if (ai->ai_family == AF_INET6) { //判断对端的地址族,是否为IP6地址族
		pr = &proto_v6; //是IP6地址族,则把初始化为IP6的proto_v6结构体附值给pr全局指针
		if (IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr))) //判断这个IP6地址是否是IP4映射而来的
			err_quit("cannot traceroute IPv4-mapped IPv6 address"); //是IP4映射的IP6地址就退出
#endif
	} else
		err_quit("unknown address family %d", ai->ai_family);

	//对端地址已经找到,地址族也已经确认,开始初始化proto结构体的剩余成员
	pr->sasend = ai->ai_addr;		/* contains destination address 对端的地址*/
	pr->sarecv = Calloc(1, ai->ai_addrlen); //用对端的地址长度,分配接收端的地址空间
	pr->salast = Calloc(1, ai->ai_addrlen); //用对端的地址长度,分配最后接收端的地址空间
	pr->sabind = Calloc(1, ai->ai_addrlen); //用对端的地址长度,分配绑定UDP套接字接口的地址空间
	pr->salen = ai->ai_addrlen; //对端的地址长度

	traceloop(); //程序主循环

	exit(0);
}
