#include	"ping.h"

void
sig_alrm(int signo)
{
	(*pr->fsend)(); //发送数据

	alarm(1); //重置信号为1秒钟之后到来
	return;
}
