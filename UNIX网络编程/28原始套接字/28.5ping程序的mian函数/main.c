#include	"ping.h"
//初始化IPv4的proto结构体
struct proto	proto_v4 = { proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP };

#ifdef	IPV6
//初始化IPv6的proto结构体
struct proto	proto_v6 = { proc_v6, send_v6, init_v6, NULL, NULL, 0, IPPROTO_ICMPV6 };
#endif

int	datalen = 56;		/* data that goes with ICMP echo request ICMP协议首部后跟的数据块长度*/

int
main(int argc, char **argv)
{
	int				c; //命令行参数选项
	struct addrinfo	*ai; //addrinfo函数所需要的结构体(次函数用来查找合适的地址)
	char *h; //用来保存已经查找到的地址的表达式形式

	//使用getopt函数来筛选命令行参数
	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "v")) != -1) {
		switch (c) {
		case 'v': //-v选项参数的设置
			verbose++;
			break;

		case '?':
			err_quit("unrecognized option: %c", c);
		}
	}

	if (optind != argc-1) //-v后面那个参数就是最后一个参数(否则报错)
		err_quit("usage: ping [ -v ] <hostname>");
	host = argv[optind]; //最后一个参数时主机名,我们获得这个主机名保存在host变量当中

	pid = getpid() & 0xffff;	/* ICMP ID field is 16 bits 算出ICMP的ID*/
	Signal(SIGALRM, sig_alrm); //注册alarm信号处理函数

	ai = Host_serv(host, NULL, 0, 0); //通过aggrinfo获得可以连接到的主机名

	h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen); //把主机名翻译成表达式,保存在变量h中
	printf("PING %s (%s): %d data bytes\n",
			ai->ai_canonname ? ai->ai_canonname : h,
			h, datalen); //打印相关消息

		/* 4initialize according to protocol */
	if (ai->ai_family == AF_INET) { //判断找到的地址的地址族是否是IPv4
		pr = &proto_v4; //pr指向IPv4结构体
#ifdef	IPV6
	} else if (ai->ai_family == AF_INET6) {//判断找到的地址的地址族是否是IPv6
		pr = &proto_v6; //pr指向IPv6结构体
		if (IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)
								 ai->ai_addr)->sin6_addr))) //判断这个IP6地址是否时由IP4地址映射而来的
			err_quit("cannot ping IPv4-mapped IPv6 address");
#endif
	} else
		err_quit("unknown address family %d", ai->ai_family);

	pr->sasend = ai->ai_addr; //把对端的地址填入pr->sasend(用来向这个地址进行发送)
	pr->sarecv = Calloc(1, ai->ai_addrlen); //用对端地址相同的大小来分配接受端的地址
	pr->salen = ai->ai_addrlen; //套接字地址的长度

	readloop();

	exit(0);
}
