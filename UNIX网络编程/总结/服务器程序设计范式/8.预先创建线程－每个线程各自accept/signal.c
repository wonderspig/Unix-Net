/* include signal */
#include	"unp.h"

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact; //创建信号处理行为的结构体

	act.sa_handler = func; //填写信号处理函数
	sigemptyset(&act.sa_mask); //设置信号处理过程中的掩码
	act.sa_flags = 0; //清空标志
	if (signo == SIGALRM) { //如果收到的是闹钟信号
#ifdef	SA_INTERRUPT //如果定义了SA_INTERRUPT宏,被中断的系统调用不重启
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x 标志中添加这个宏*/
#endif
	} else { //进入此分支代表不是闹钟信号
#ifdef	SA_RESTART //如果定义了中断系统调用重启标志
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD 加入这个标志 */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) //把填写完的信号行为结构体设置信号处理程序
		return(SIG_ERR); //失败返回出错
	return(oact.sa_handler); //成功返回原来的信号处理程序的函数指针
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}
