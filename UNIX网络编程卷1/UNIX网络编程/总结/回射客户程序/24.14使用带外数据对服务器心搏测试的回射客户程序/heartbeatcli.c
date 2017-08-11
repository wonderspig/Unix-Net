#include	"unp.h"

static int		servfd;
static int		nsec;			/* #seconds betweeen each alarm */
static int		maxnprobes;		/* #probes w/no response before quit */
static int		nprobes;		/* #probes since last server response */
static void	sig_urg(int), sig_alrm(int);

void
heartbeat_cli(int servfd_arg, int nsec_arg, int maxnprobes_arg)
{
	servfd = servfd_arg;		/* set globals for signal handlers 连接到服务器的套接字*/
	if ( (nsec = nsec_arg) < 1) //两次alarm信号的间隔不能小于1秒(轮询向服务器发送带外数据的间隔)
		nsec = 1;
	if ( (maxnprobes = maxnprobes_arg) < nsec) //最大的重试次数
		maxnprobes = nsec; //最大重试次数不小于间隔秒数
	nprobes = 0; //已经重试发送带外数据的次数
	//SIGURG信号为收到带外数据时发送的信号
	Signal(SIGURG, sig_urg); //注册SIGURG信号处理函数
	Fcntl(servfd, F_SETOWN, getpid()); //设置套接字属主为本进程

	Signal(SIGALRM, sig_alrm); //注册alarm信号处理函数
	alarm(nsec); //nsec秒后发送alarm信号
}

static void
sig_urg(int signo)
{
	int		n;
	char	c;
 
	if ( (n = recv(servfd, &c, 1, MSG_OOB)) < 0) { //接收带外数据
		if (errno != EWOULDBLOCK)
			err_sys("recv error");
	}
	//有没有收到带外数据无所谓,只要确认对端确实发送了带外数据,就说明对端还存活
	nprobes = 0;			/* reset counter 重置已经发送带外数据的次数为0*/
	return;					/* may interrupt client code */
}

static void
sig_alrm(int signo)
{
	if (++nprobes > maxnprobes) { //把当前发送带外数据的次数加1,并且和最大重试次数比较
		fprintf(stderr, "server is unreachable\n"); //已经超过最大重试次数,对端已经不存在
		exit(0);
	}
	Send(servfd, "1", 1, MSG_OOB); //发送一个字节带外数据
	alarm(nsec); //重新设置间隔秒数
	return;					/* may interrupt client code */
}
