#include	"unp.h"

static int	servfd;
static int	nsec;			/* #seconds between each alarm */
static int	maxnalarms;		/* #alarms w/no client probe before quit */
static int	nprobes;		/* #alarms since last client probe */
static void	sig_urg(int), sig_alrm(int);

void
heartbeat_serv(int servfd_arg, int nsec_arg, int maxnalarms_arg)
{
	servfd = servfd_arg;		/* set globals for signal handlers 连接到服务器的套接字 */
	if ( (nsec = nsec_arg) < 1) //两次发送带外数据间隔的秒数,不能小于1秒
		nsec = 1; 
	if ( (maxnalarms = maxnalarms_arg) < nsec) //最大的重试次数,不能小于间隔秒数
		maxnalarms = nsec;
	//SIGURG信号为带外数据到达时收到的信号
	Signal(SIGURG, sig_urg); //注册SIGURG信号处理程序
	Fcntl(servfd, F_SETOWN, getpid()); //设置套接字的属主进程为本进程(信号会发送给套接字的属主进程)

	Signal(SIGALRM, sig_alrm); //注册SIGALRM信号处理程序
	alarm(nsec); //设置nsec秒收收到alarm信号
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
	//收没收到带外数据无所谓,收到此信号确认对端已经发送
	Send(servfd, &c, 1, MSG_OOB);	/* echo back out-of-band byte 发还给对端一个带外数据*/

	nprobes = 0;			/* reset counter 对端还存活,已经发送带外数据的次数重置为0 */
	return;					/* may interrupt server code */
}

static void
sig_alrm(int signo)
{
	if (++nprobes > maxnalarms) { //已经发送带外数据的次数加1,且和最大发送次数比较
		printf("no probes from client\n"); //超过最大发送次数说明对端已经不存在
		exit(0);
	}
	alarm(nsec); //重置计时器到时时间
	return;					/* may interrupt server code */
}
