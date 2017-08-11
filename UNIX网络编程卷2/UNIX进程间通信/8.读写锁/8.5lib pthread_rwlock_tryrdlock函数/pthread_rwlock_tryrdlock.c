/* include tryrdlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_tryrdlock(pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC) //检查该读写锁结构体是否已经初始化
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0) //加锁互斥量
		return(result);

	if (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) //如果有线程占用着写锁,或者有欲加写锁的线程正在阻塞,直接出错
		result = EBUSY;			/* held by a writer or waiting writers */
	else
		rw->rw_refcount++;		/* increment count of reader locks 否则加锁成功,占有读锁的计数器+1 */

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end tryrdlock */

int
Pthread_rwlock_tryrdlock(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_tryrdlock(rw)) != 0) {
		errno = n;
		err_sys("pthread_rwlock_tryrdlock error");
	}
	return(n);
}
