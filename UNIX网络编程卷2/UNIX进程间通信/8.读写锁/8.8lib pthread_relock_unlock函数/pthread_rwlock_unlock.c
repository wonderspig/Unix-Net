/* include unlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_unlock(pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC) //检查该读写锁结构体是否已经初始化
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0) //加锁互斥量
		return(result);

	if (rw->rw_refcount > 0) //如果当前锁被n个读锁占有
		rw->rw_refcount--;			/* releasing a reader 读锁数量-1*/
	else if (rw->rw_refcount == -1) //如果当前锁被写锁占有
		rw->rw_refcount = 0;		/* releasing a reader 当前锁状态为没有任何锁占有,因为写锁只有一把*/
	else
		err_dump("rw_refcount = %d", rw->rw_refcount); //别的状态则出错

	 
		/* 4give preference to waiting writers over waiting readers */
	if (rw->rw_nwaitwriters > 0) { //如果有申请写锁线程阻塞
		if (rw->rw_refcount == 0) //并且现在的读写锁没有任何线程占有
			result = pthread_cond_signal(&rw->rw_condwriters); //唤醒写锁条件变量
	} else if (rw->rw_nwaitreaders > 0) //如果有申请读锁线程阻塞
		result = pthread_cond_broadcast(&rw->rw_condreaders); //唤醒所有申请读锁而阻塞的线程

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end unlock */

void
Pthread_rwlock_unlock(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_unlock(rw)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_unlock error");
}
