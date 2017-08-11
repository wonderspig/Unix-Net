/* include sem_getvalue */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_getvalue(mysem_t *sem, int *pvalue)
{
	int		n;

	if (sem->sem_magic != SEM_MAGIC) { 	//检查互斥量结构体是否初始化
		errno = EINVAL;
		return(-1);
	}

	if ( (n = pthread_mutex_lock(&sem->sem_mutex)) != 0) { //加锁互斥量
		errno = n;
		return(-1);
	}
	*pvalue = sem->sem_count; //把互斥量的值存入pvalue指针指向的内存
	pthread_mutex_unlock(&sem->sem_mutex); //解锁互斥量
	return(0);
}
/* end sem_getvalue */

void
Mysem_getvalue(mysem_t *sem, int *pvalue)
{
	if (mysem_getvalue(sem, pvalue) == -1)
		err_sys("mysem_getvalue error");
}

