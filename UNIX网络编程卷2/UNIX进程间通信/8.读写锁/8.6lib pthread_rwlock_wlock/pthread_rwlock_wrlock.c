/* include wrlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_wrlock(pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC) //检查该读写锁结构体是否已经初始化
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0) //加锁互斥量
		return(result);

	while (rw->rw_refcount != 0) { //如果当前的读写锁有线程占有,不管读占有,还是写占有
		rw->rw_nwaitwriters++; //阻塞在写锁的线程计数+1
		result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex); //睡眠等待条件变量
		rw->rw_nwaitwriters--; //阻塞在写锁的计数器_1
		if (result != 0) //如果是条件变量失败,则退出循环
			break;
	}
	if (result == 0) //查看是否是因为条件变量失败而退出的循环,如果是正常退出
		rw->rw_refcount = -1; //就把锁的状态改为已经由写锁占有

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end wrlock */

void
Pthread_rwlock_wrlock(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_wrlock(rw)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_wrlock error");
}
