/* include sem_trywait */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_trywait(mysem_t *sem)
{
	int		n, rc;

	if (sem->sem_magic != SEM_MAGIC) { //检查信号量结构体是否经过初始化
		errno = EINVAL;
		return(-1);
	}

	if ( (n = pthread_mutex_lock(&sem->sem_mutex)) != 0) { //加锁信号量
		errno = n;
		return(-1);
	}
	if (sem->sem_count > 0) { //如果信号量的值大于0
		sem->sem_count--; //把信号量的值-1
		rc = 0; //返回值
	} else {
		rc = -1; //返回值
		errno = EAGAIN; //设置errno的值
	}
	pthread_mutex_unlock(&sem->sem_mutex); //解锁互斥量
	return(rc); //返回相应的值
}
/* end sem_trywait */

int
Mysem_trywait(mysem_t *sem)
{
	int		rc;

	if ( (rc = mysem_trywait(sem)) == -1 && errno != EAGAIN)
		err_sys("mysem_trywait error");
	return(rc);
}
