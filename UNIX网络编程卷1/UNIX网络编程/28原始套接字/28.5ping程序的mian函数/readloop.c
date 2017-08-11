#include	"ping.h"

void
readloop(void)
{
	int				size;
	char			recvbuf[BUFSIZE];
	char			controlbuf[BUFSIZE];
	struct msghdr	msg;
	struct iovec	iov;
	ssize_t			n;
	struct timeval	tval;

	//pr已经指向了正确的结构体IP4或者IP6
	//用addinfo查找到的地址的地址族,pr指向的结构体所对应的ICMP协议,创建原始套接字
	sockfd = Socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
	setuid(getuid());		/* don't need special permissions any more 创建完原始套接字之后,不再需要root权限*/
	if (pr->finit) //如果pr指向的结构体为IP6版本,那么此判断式为true
		(*pr->finit)(); //调用IP6的初始化函数(设置过滤器,套接字选项等等参数)

	size = 60 * 1024;		/* OK if setsockopt fails 接收缓冲区大小*/
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)); //通过套接字选项,设置接受缓冲区

	sig_alrm(SIGALRM);		/* send first packet 直接调用信号处理函数,次函数中向对端addrinfo地址发送一个数据包,且设置下一个alarm值*/

	iov.iov_base = recvbuf; //接受缓冲区
	iov.iov_len = sizeof(recvbuf); //接收缓冲区大小
	msg.msg_name = pr->sarecv; //对端的协议地址
	msg.msg_iov = &iov; //接收缓冲区结构体指针
	msg.msg_iovlen = 1; //接受缓冲区结构体的个数
	msg.msg_control = controlbuf; //辅助数据缓冲区指针
	for ( ; ; ) { //无限循环中接收对端传来的数据
		msg.msg_namelen = pr->salen; //对端协议地址的长度
		msg.msg_controllen = sizeof(controlbuf); //辅助数据缓冲区的长度
		n = recvmsg(sockfd, &msg, 0); //从对端接收数据
		if (n < 0) { //出错
			if (errno == EINTR) //是被信号打断的出错
				continue; //继续循环
			else
				err_sys("recvmsg error"); //别的出错是不可接受的
		}

		Gettimeofday(&tval, NULL); //获得当前时间(接收完数据的时间点)
		(*pr->fproc)(recvbuf, n, &msg, &tval); //用保存在pr结构体中的对应处理函数(IP4或IP6),来处理接受缓冲区中的数据,并用时间点来算出RTT
	}
}
