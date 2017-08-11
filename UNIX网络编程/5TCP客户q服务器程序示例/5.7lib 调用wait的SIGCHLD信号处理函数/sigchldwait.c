#include	"unp.h"

void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	pid = wait(&stat); //等待子进程
	printf("child %d terminated\n", pid); //打印子进程PID
	return;
}
