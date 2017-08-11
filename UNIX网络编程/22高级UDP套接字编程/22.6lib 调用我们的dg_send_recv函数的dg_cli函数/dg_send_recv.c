/* include dgsendrecv1 */
#include	"unprtt.h"
#include	<setjmp.h>

#define	RTT_DEBUG

static struct rtt_info   rttinfo;
static int	rttinit = 0;
static struct msghdr	msgsend, msgrecv;	/* assumed init to 0 把这个结构初始化为0 */
static struct hdr { //信息的控制头,有序列号和时间戳,因为在全局静态变量中,所以初始值都是0
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;		/* timestamp when sent */
} sendhdr, recvhdr;

static void	sig_alrm(int signo);
static sigjmp_buf	jmpbuf;

ssize_t
dg_send_recv(int fd, const void *outbuff, size_t outbytes,
			 void *inbuff, size_t inbytes,
			 const SA *destaddr, socklen_t destlen)
{
	ssize_t			n;
	struct iovec	iovsend[2], iovrecv[2];
	//在首次调用本函数时,使用rtt_init进行初始化
	if (rttinit == 0) {
		rtt_init(&rttinfo);		/* first time we're called */
		//设置标志,说明已经初始化过了,本函数不是首次调用了
		rttinit = 1;
		rtt_d_flag = 1;
	}

	sendhdr.seq++; //自增1
	//填写发送的msghdr结构体
	msgsend.msg_name = (void*)destaddr; //发送目的地址
	msgsend.msg_namelen = destlen; //地址结构体长度
	msgsend.msg_iov = iovsend; //发送缓冲区的指针数组
	msgsend.msg_iovlen = 2; //数组里的成员个数
	//2个数组成员
	iovsend[0].iov_base = &sendhdr; //一个是信息头,里面有序列号和时间戳
	iovsend[0].iov_len = sizeof(struct hdr);
	iovsend[1].iov_base = (void*)outbuff; //一个是信息正文,由调用者提供,传参进来的
	iovsend[1].iov_len = outbytes;
	//填写接收的msghdr结构体
	msgrecv.msg_name = NULL;
	msgrecv.msg_namelen = 0;
	msgrecv.msg_iov = iovrecv; //接收缓冲区的指针数组
	msgrecv.msg_iovlen = 2; //数组里的成员个数
	//2个数组成员
	iovrecv[0].iov_base = &recvhdr; //一个时信息头,里面有序列号和时间戳
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = inbuff;//一个是信息正文,由调用者提供,传参进来的
	iovrecv[1].iov_len = inbytes;
/* end dgsendrecv1 */

/* include dgsendrecv2 */
	Signal(SIGALRM, sig_alrm); //注册SIGALRM的处理程序
	//把崇川计数器设置为0
	rtt_newpack(&rttinfo);		/* initialize for this packet 初始化rtt_info结构体 */

sendagain:
#ifdef	RTT_DEBUG //RTT_DEBUG表示调试状态,则打印如下调试信息,打印发送数据报的序列号
	fprintf(stderr, "send %4d: ", sendhdr.seq);
#endif
	sendhdr.ts = rtt_ts(&rttinfo); //填写发送数据头信息中的时间戳(把当前时间戳填入)
	Sendmsg(fd, &msgsend, 0); //发送数据
	//设置alarm信号
	alarm(rtt_start(&rttinfo));	/* calc timeout value & start timer 设置超时时间,以秒为单位*/
#ifdef	RTT_DEBUG //RTT_DEBUG表示调试状态
	rtt_debug(&rttinfo);
#endif

	if (sigsetjmp(jmpbuf, 1) != 0) { //设置信号处理程序跳转回来的位置
		//这里的代码在信号处理程序跳转过来的时候执行,已经发生超时
		//计算下一个超时时间值,更新结构体rttinfo中的数据
		//若放弃返回-1,还能继续增加超时时间则返回0
		if (rtt_timeout(&rttinfo) < 0) { 
			//超时时间过长,放弃的处理
			err_msg("dg_send_recv: no response from server, giving up");
			rttinit = 0;	/* reinit in case we're called again */
			errno = ETIMEDOUT;
			return(-1);
		}
#ifdef	RTT_DEBUG //RTT_DEBUG表示调试状态
		err_msg("dg_send_recv: timeout, retransmitting");
#endif
		goto sendagain; //返回继续设置超时值,然后发送请求
	}

	do {
		n = Recvmsg(fd, &msgrecv, 0); //接收UDP数据报,填写msgrecv结构
#ifdef	RTT_DEBUG //RTT_DEBUG表示调试状态,打印接收消息的序列号
		fprintf(stderr, "recv %4d\n", recvhdr.seq);
#endif
	} while (n < sizeof(struct hdr) || recvhdr.seq != sendhdr.seq); //判断序列号是否相符
	//正常接收数据,且序列号也没问题,注销alarm信号
	alarm(0);			/* stop SIGALRM timer */
		/* 4calculate & store new RTT estimator values */
	//更新RTT估算因子
	//第二个参数为当前的RTT,用它计算出估算因子,把它更新进rttinfo结构体
	//rtt_ts返回当前的时间戳,减去接收消息中的时间戳(接收消息中的时间戳是发送时的时间戳),就是来回服务器的时间RTT
	rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recvhdr.ts);

	return(n - sizeof(struct hdr));	/* return size of received datagram 返回接收到的数据字节数(去掉头信息中的序列号和时间戳的字节数)*/
}

static void
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);
}
/* end dgsendrecv2 */

ssize_t
Dg_send_recv(int fd, const void *outbuff, size_t outbytes,
			 void *inbuff, size_t inbytes,
			 const SA *destaddr, socklen_t destlen)
{
	ssize_t	n;

	n = dg_send_recv(fd, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen);
	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}
