/* include func */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

/* include rwlock_cancelwrwait */
static void
rwlock_cancelwrwait(void *arg)
{
	pthread_rwlock_t	*rw;

	rw = arg;
	rw->rw_nwaitwriters--; //读写锁计数器中等待加写锁的计数器-1
	pthread_mutex_unlock(&rw->rw_mutex);
}
/* end rwlock_cancelwrwait */

int
pthread_rwlock_wrlock(pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) == -1)
		return(result);

	while (rw->rw_refcount != 0) {
		rw->rw_nwaitwriters++;
		pthread_cleanup_push(rwlock_cancelwrwait, (void *) rw);
		result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
		pthread_cleanup_pop(0);
		rw->rw_nwaitwriters--;
		if (result != 0)
			break;
	}
	if (result == 0)
		rw->rw_refcount = -1;

	pthread_mutex_unlock(&rw->rw_mutex);
	return(0);
}
/* end func */

void
Pthread_rwlock_wrlock(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_wrlock(rw)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_wrlock error");
}
