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

	if (argc != 2)
		err_quit("usage: mqnotifysig3 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK); //指定非阻塞标志
	Mq_getattr(mqd, &attr);
	buff = Malloc(attr.mq_msgsize);

	Sigemptyset(&zeromask);		/* no signals blocked */
	Sigemptyset(&newmask);
	Sigemptyset(&oldmask);
	Sigaddset(&newmask, SIGUSR1);
		/* 4establish signal handler, enable notification */
	Signal(SIGUSR1, sig_usr1);
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGUSR1;
	Mq_notify(mqd, &sigev);

	for ( ; ; ) {
		Sigprocmask(SIG_BLOCK, &newmask, &oldmask);	/* block SIGUSR1 */
		while (mqflag == 0)
			sigsuspend(&zeromask);
		mqflag = 0;		/* reset flag */

		Mq_notify(mqd, &sigev);			/* reregister first */
		while ( (n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) { //用循环读取消息队列,读到出错为止
			printf("read %ld bytes\n", (long) n);
		}
		if (errno != EAGAIN) //错误是队列中没有消息了(此错误可以接受)
			err_sys("mq_receive error");
		Sigprocmask(SIG_UNBLOCK, &newmask, NULL);	/* unblock SIGUSR1 */
	}
	exit(0);
}

static void
sig_usr1(int signo)
{
	mqflag = 1;
	return;
}
