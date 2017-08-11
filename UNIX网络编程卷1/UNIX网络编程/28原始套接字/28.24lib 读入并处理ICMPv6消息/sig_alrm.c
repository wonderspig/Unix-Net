#include	"trace.h"

int gotalarm; //是否进入过此信号处理函数的标志量

void
sig_alrm(int signo)
{
	gotalarm = 1;	/* set flag to note that alarm occurred 把标志量置1*/
	return;			/* and interrupt the recvfrom() */
}
