/* include sem_post */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_post(mysem_t *sem)
{
	int		n;

	if (sem->sem_magic != SEM_MAGIC) { //检查信号量结构体是否初始化过
		errno = EINVAL;
		return(-1);
	}

	if ( (n = pthread_mutex_lock(&sem->sem_mutex)) != 0) { //加锁互斥量
		errno = n;
		return(-1);
	}
	if (sem->sem_count == 0) //如果当前信号量的值为0,则唤醒条件变量
		pthread_cond_signal(&sem->sem_cond);
	sem->sem_count++; //信号量的值+1
	pthread_mutex_unlock(&sem->sem_mutex); //解锁互斥量
	return(0);
}
/* end sem_post */

void
Mysem_post(mysem_t *sem)
{
	if (mysem_post(sem) == -1)
		err_sys("mysem_post error");
}
