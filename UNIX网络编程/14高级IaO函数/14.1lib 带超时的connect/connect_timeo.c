/* include connect_timeo */
#include	"unp.h"

static void	connect_alarm(int);

int
connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc	*sigfunc; //信号处理函数Signal的第二个参数,和返回值
	int		n;

	sigfunc = Signal(SIGALRM, connect_alarm); //注册定时器alarm的信号处理函数为connect_alarm
	if (alarm(nsec) != 0) //调用alarm函数发送alarm信号
		err_msg("connect_timeo: alarm was already set"); //已经推翻了以前设置的alarm信号设置

	if ( (n = connect(sockfd, saptr, salen)) < 0) { //调用connect连接服务器
		close(sockfd); //失败了就关闭套接字(防止三路握手继续进行),并设置error值
		if (errno == EINTR)
			errno = ETIMEDOUT;
	}
	alarm(0);					/* turn off the alarm 取消alarm信号 */
	Signal(SIGALRM, sigfunc);	/* restore previous signal handler 把alarm信号改为原来的处理函数 */

	return(n);
}

static void
connect_alarm(int signo) //打断connect调用后直接返回
{
	return;		/* just interrupt the connect() */
}
/* end connect_timeo */

void
Connect_timeo(int fd, const SA *sa, socklen_t salen, int sec)
{
	if (connect_timeo(fd, sa, salen, sec) < 0)
		err_sys("connect_timeo error");
}
