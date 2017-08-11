/* include signal_rt */
#include	"unpipc.h"

Sigfunc_rt *
signal_rt(int signo, Sigfunc_rt *func)
{
	struct sigaction	act, oact;

	act.sa_sigaction = func;		/* must store function addr here 指定SA_SIGINFO标志后,信号处理程序由sa_sigaction这个成员设置*/
	sigemptyset(&act.sa_mask); //信号处理程序执行期间,不阻塞任何信号
	act.sa_flags = SA_SIGINFO;		/* must specify this for realtime 设置SA_SIGINFO,用来注册实时信号处理程序 */
	if (signo == SIGALRM) { //如果信号是SIGALRM(我们打算不重启被SIGALRM打断的信号)
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x SA_INTERRUPT此标志为不重启被中断的系统调用 */
#endif
	} else { //此分支代表不是SIGALRM信号
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD 重启被中断的系统调用*/
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) //注册信号处理程序
		return((Sigfunc_rt *) SIG_ERR); //注册信号处理函数失败,则返回错误
	return(oact.sa_sigaction); //返回原来的信号处理函数
}
/* end signal_rt */

Sigfunc_rt *
Signal_rt(int signo, Sigfunc_rt *func)
{
	Sigfunc_rt	*sigfunc;

	if ( (sigfunc = signal_rt(signo, func)) == (Sigfunc_rt *) SIG_ERR)
		err_sys("signal_rt error");
	return(sigfunc);
}
