#include	"unpipc.h"

static void	sig_rt(int, siginfo_t *, void *);

int
main(int argc, char **argv)
{
	int		i, j;
	pid_t	pid;
	sigset_t	newset;
	union sigval	val;

	printf("SIGRTMIN = %d, SIGRTMAX = %d\n", (int) SIGRTMIN, (int) SIGRTMAX); //打印实时信号的值的范围
	//创建子进程
	if ( (pid = Fork()) == 0) { //在子进程中
			/* 4child: block three realtime signals */
		Sigemptyset(&newset); //清空newset信号集
		Sigaddset(&newset, SIGRTMAX); //加入一个实时信号(信号值从大到小,期待信号值小的信号先被递交)
		Sigaddset(&newset, SIGRTMAX - 1); //加入一个实时信号(信号值从大到小,期待信号值小的信号先被递交)
		Sigaddset(&newset, SIGRTMAX - 2); //加入一个实时信号(信号值从大到小,期待信号值小的信号先被递交)
		Sigprocmask(SIG_BLOCK, &newset, NULL); //阻塞这三个实时信号

			/* 4establish signal handler with SA_SIGINFO set */
		//注册这三个实时信号的信号处理程序(用实时的信号函数Signal_rt来注册)
		Signal_rt(SIGRTMAX, sig_rt); 
		Signal_rt(SIGRTMAX - 1, sig_rt);
		Signal_rt(SIGRTMAX - 2, sig_rt);

		sleep(6);		/* let parent send all the signals 睡6秒(等待父进程发送预定的9个实时信号) */

		Sigprocmask(SIG_UNBLOCK, &newset, NULL);	/* unblock 解除这6个实时信号的阻塞,期待他们会按照实时信号的规则排队递交 */
		sleep(3);		/* let all queued signals be delivered 睡三秒等待信号处理程序打印9个printf */
		exit(0);
	}
	//在父进程中
		/* 4parent sends nine signals to child */
	sleep(3);		/* let child block all signals */
	for (i = SIGRTMAX; i >= SIGRTMAX - 2; i--) { //从大到小循环遍历实时信号的最大的三个值
		for (j = 0; j <= 2; j++) { //j为每个实时信号的序号,因为每个实时信号发送三次,所以是0,1,2
			val.sival_int = j; //sigval为发送实时信号的函数所需要的结构体,填入这个结构体需要发送的信号的附加值
			Sigqueue(pid, i, val); //发送这个实时信号给pid,信号值为i,信号所附带的信息为val
			printf("sent signal %d, val = %d\n", i, j); //打印发送的这条信号的消息
		}
	}
	exit(0);
}
//信号处理函数
static void
sig_rt(int signo, siginfo_t *info, void *context)
{
	printf("received signal #%d, code = %d, ival = %d\n",
		   signo, info->si_code, info->si_value.sival_int); //打印收到的信号,信号由什么产生的,信号的附加消息
}
