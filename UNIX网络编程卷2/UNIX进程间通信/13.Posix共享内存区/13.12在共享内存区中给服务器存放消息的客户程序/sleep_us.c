#include	"unpipc.h"

int
sleep_us(unsigned int nusecs)
{
	struct timeval	tval;

	if (nusecs == 0)
		return(0);

	for ( ; ; ) {
		tval.tv_sec = nusecs / 1000000;
		tval.tv_usec = nusecs % 1000000;
		if (select(0, NULL, NULL, NULL, &tval) == 0)
			return(0);		/* all OK */
		/*
		 * Note than on an interrupted system call there's not
		 * much we can do, since the timeval{} isn't updated with the time
		 * remaining.  We could obtain the clock time before the call, and
		 * then obtain the clock time here, subtracting them to determine
		 * how long select() blocked before it was interrupted, but that
		 * seems like too much work :-)
		 */
		if (errno != EINTR)
			return(-1);
		/* else go around again */
	}
}

void
Sleep_us(unsigned int nusecs)
{
	if (sleep_us(nusecs) == -1)
		err_sys("sleep_us error");
}
