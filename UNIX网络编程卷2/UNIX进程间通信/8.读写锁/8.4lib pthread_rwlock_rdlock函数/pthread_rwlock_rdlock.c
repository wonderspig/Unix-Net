/* include rdlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_rdlock(pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC) //首先判断读写锁结构体有没有被初始化
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0) //加锁互斥量
		return(result);

		/* 4give preference to waiting writers */
	while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) { //如果被写锁占用,或者有期待加写锁的线程阻塞
		rw->rw_nwaitreaders++; //加写锁的线程计数+1
		result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex); //读锁条件变量等待
		rw->rw_nwaitreaders--;
		if (result != 0)
			break; //条件变量失败而退出
	}
	if (result == 0) //查看上一个循环是否是因为条件变量失败而退出的,如果正常退出
		rw->rw_refcount++;		/* another reader has a read lock 计数器+1,代表当前占有读锁的线程+1 */

	pthread_mutex_unlock(&rw->rw_mutex); //解锁互斥量
	return (result);
}
/* end rdlock */

void
Pthread_rwlock_rdlock(pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = pthread_rwlock_rdlock(rw)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_rdlock error");
}
