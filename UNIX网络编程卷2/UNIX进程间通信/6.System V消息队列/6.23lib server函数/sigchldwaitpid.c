#include	"unpipc.h"

void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0); //等待任意子进程结束并回首资源
	return;
}
