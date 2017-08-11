#include	"unp.h"

void
tv_sub(struct timeval *out, struct timeval *in) //计算两个时间点之差
{
	if ( (out->tv_usec -= in->tv_usec) < 0) {	/* out -= in */
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
