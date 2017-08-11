/* include destroy */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_destroy(pthread_rwlock_t *rw)
{
	if (rw->rw_magic != RW_MAGIC) //如果标志不是该读写锁已使用,则出错
		return(EINVAL);
	if (rw->rw_refcount != 0 ||
		rw->rw_nwaitreaders != 0 || rw->rw_nwaitwriters != 0) //rw->rw_refcount != 0表示有线程在占用读写锁
											//rw->rw_nwaitreaders != 0 || rw->rw_nwaitwriters != 0表示有读线程或写线程阻塞在加锁上,则报错
		return(EBUSY);

	pthread_mutex_destroy(&rw->rw_mutex); //销毁互斥量
	pthread_cond_destroy(&rw->rw_condreaders); //销毁读条件变量
	pthread_cond_destroy(&rw->rw_condwriters); //销毁写条件变量
	rw->rw_magic = 0; //标志置为0

	return(0);
}
/* end destroy */

void
Pthread_rwlock_destroy(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_destroy(rw)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_destroy error");
}
