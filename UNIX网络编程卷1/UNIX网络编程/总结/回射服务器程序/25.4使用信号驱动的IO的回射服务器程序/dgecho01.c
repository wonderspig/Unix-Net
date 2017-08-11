/* include dgecho1 */
#include	"unp.h"

static int		sockfd;

#define	QSIZE	   8		/* size of input queue DG数组的元素个数 */
#define	MAXDG	4096		/* max datagram size 存放数据正文的缓冲区的大小*/
//DG数组
typedef struct {
  void		*dg_data;		/* ptr to actual datagram 指向收取数据报的一个指针 */
  size_t	dg_len;			/* length of datagram 数据报的长度 */
  struct sockaddr  *dg_sa;	/* ptr to sockaddr{} w/client's address 指向客户协议地址的指针 */
  socklen_t	dg_salen;		/* length of sockaddr{} 客户协议地址的长度*/
} DG;
static DG	dg[QSIZE];			/* queue of datagrams to process 用DG类型声明一个数组*/
static long	cntread[QSIZE+1];	/* diagnostic counter 诊断计数器 */

static int	iget;		/* next one for main loop to process 主循环将处理的下一个数组元素的下标(处理完了队列数-1)*/
static int	iput;		/* next one for signal handler to read into 信号处理函数将处理的下一个数组元素的下标(往队列中放入一个) */
static int	nqueue;		/* # on queue for main loop to process 队列中提供主循环处理的数据报的总数*/
static socklen_t clilen;/* max length of sockaddr{} */

static void	sig_io(int);
static void	sig_hup(int);
/* end dgecho1 */

/* include dgecho2 */
void
dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t clilen_arg)
{
	int			i;
	const int	on = 1;
	sigset_t	zeromask, newmask, oldmask;

	sockfd = sockfd_arg; //连接到客户端的套接字存放在全局变量中(要提供给信号处理程序使用)
	clilen = clilen_arg; //客户端地址长度保存在全局变量中

	for (i = 0; i < QSIZE; i++) {	/* init queue of buffers */
		dg[i].dg_data = Malloc(MAXDG);
		dg[i].dg_sa = Malloc(clilen);
		dg[i].dg_salen = clilen;
	}
	//队列中供主循环处理的数据报总数初始化为0，主循环下一个要处理的数组元素下标为0,信号处理函数要处理的下一个数组元素下标为0
	iget = iput = nqueue = 0; 

	Signal(SIGHUP, sig_hup); //注册SIGHUP信号处理函数(挂起信号)
	Signal(SIGIO, sig_io); //注册SIGIO信号处理函数(信号驱动式IO提供的信号)
	Fcntl(sockfd, F_SETOWN, getpid()); //设置套接字属主为本进程
	Ioctl(sockfd, FIOASYNC, (void*)&on); //打开套接字的信号驱动式IO(可以使用fcntl的O_ASYNC)
	Ioctl(sockfd, FIONBIO, (void*)&on); //打开套接字非阻塞式IO(可以使用fcntl的O_NONBLOCK)
	//清空zeromask,oldmask,newmask信号集
	Sigemptyset(&zeromask);		/* init three signal sets */
	Sigemptyset(&oldmask);
	Sigemptyset(&newmask);
	Sigaddset(&newmask, SIGIO);	/* signal we want to block 把SIGIO信号添加入newmask信号集 */

	Sigprocmask(SIG_BLOCK, &newmask, &oldmask); //阻塞SIGIO信号,保存当前的信号掩码到oldmask中
	for ( ; ; ) {
		while (nqueue == 0) //测试当前提供主循环处理的数据报总数
			//为0,则进程无事可做,不阻塞任何信号的进入休眠,等待信号到来
			sigsuspend(&zeromask);	/* wait for datagram to process */

			/* 4unblock SIGIO */
		Sigprocmask(SIG_SETMASK, &oldmask, NULL); //解除阻塞SIGIO信号
		//想客户程序发送应答
		//发送到连接客户的套接字,此处处理数组主循环下标标记的元素
		Sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0,
			   dg[iget].dg_sa, dg[iget].dg_salen);

		if (++iget >= QSIZE) //主循环下标+1(处理下一个数组元素)
			iget = 0; //如果下标越界,则重新从0开始

			/* 4block SIGIO */
		Sigprocmask(SIG_BLOCK, &newmask, &oldmask); //继续阻塞SIGIO信号
		nqueue--; //处理完了,队列中元素-1
	}
}
/* end dgecho2 */

/* include sig_io */
static void
sig_io(int signo)
{
	ssize_t		len;
	int			nread;
	DG			*ptr;

	for (nread = 0; ; ) {
		if (nqueue >= QSIZE) //如果DG数组已满
			err_quit("receive overflow");

		ptr = &dg[iput]; //ptr指向"当前信号处理函数应该处理的下标元素"处
		ptr->dg_salen = clilen; //把全局变量中的客户地址长度再赋值一次
		len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0,
					   ptr->dg_sa, &ptr->dg_salen); //接收数据报(套接字非阻塞)
		if (len < 0) {
			if (errno == EWOULDBLOCK) //读到缓冲区中没有数据为止才退出循环(处理信号不排队的问题)
				break;		/* all done; no more queued to read */
			else
				err_sys("recvfrom error");
		}
		ptr->dg_len = len; //数据长度填入DG数组

		nread++; //诊断计数器(当break循环之后,此数代表本次信号处理程序加入了多少个数据报到队列中)
		nqueue++; //队列中的数据+1(主循环处理后再-1)
		if (++iput >= QSIZE) //信号处理函数处理的下标+1,越界则置0
			iput = 0;

	}
	cntread[nread]++;		/* histogram of # datagrams read per signal 诊断计数器*/
}
/* end sig_io */

/* include sig_hup */
static void
sig_hup(int signo)
{
	int		i;

	for (i = 0; i <= QSIZE; i++)
		//读出诊断计数器的结果
		//i为1代表的元素为读出一个数据报的次数。。。
		printf("cntread[%d] = %ld\n", i, cntread[i]); 
}
/* end sig_hup */
