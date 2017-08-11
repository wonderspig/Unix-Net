#include	"unpipc.h"

volatile sig_atomic_t	mqflag;		/* set nonzero by signal handler */
static void	sig_usr1(int);

int
main(int argc, char **argv)
{
	mqd_t	mqd;
	void	*buff;
	ssize_t	n;
	sigset_t	zeromask, newmask, oldmask;
	struct mq_attr	attr;
	struct sigevent	sigev;

	if (argc != 2) //如果命令行参数不为1,则出错
		err_quit("usage: mqnotifysig2 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY); //打开命令行参数所指定的消息队列
	Mq_getattr(mqd, &attr); //获得该消息队列的属性
	buff = Malloc(attr.mq_msgsize); //通过消息队列属性中每条消息的最大大小来动态分配缓冲区

	Sigemptyset(&zeromask);		/* no signals blocked 清空信号集zeromask */
	Sigemptyset(&newmask); //清空信号集newmask
	Sigemptyset(&oldmask); //清空信号集oldmask
	Sigaddset(&newmask, SIGUSR1); //把SIGUSR1信号设置进newmask

		/* 4establish signal handler, enable notification */
	Signal(SIGUSR1, sig_usr1); //注册SIGUSR1信号处理函数
	sigev.sigev_notify = SIGEV_SIGNAL; //以信号的方式接收通知
	sigev.sigev_signo = SIGUSR1; //发出通知的信号为SIGUSR1
	Mq_notify(mqd, &sigev); //把当前进程注册为接收该队列的通知

	for ( ; ; ) {
		Sigprocmask(SIG_BLOCK, &newmask, &oldmask);	/* block SIGUSR1 阻塞SIGUSR1信号 */
		while (mqflag == 0) //检查全局标志量 
			sigsuspend(&zeromask); //原子的解除对信号SIGUSR1的阻塞并进入休眠等待这个信号到来
		mqflag = 0;		/* reset flag 充值全局标志量*/

		Mq_notify(mqd, &sigev);			/* reregister first 在从消息队列中取消息之前,先再次把当前进程注册为接收该队列的通知 */
		n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL); //从消息队列中取消息
		printf("read %ld bytes\n", (long) n); //打印这个消息的大小
		Sigprocmask(SIG_UNBLOCK, &newmask, NULL);	/* unblock SIGUSR1 解除对SIGUSR1的阻塞*/
	}
	exit(0);
}

static void
sig_usr1(int signo)
{
	mqflag = 1;
	return;
}
