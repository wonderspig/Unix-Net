/* include sem_close */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_close(mysem_t *sem)
{
	if (sem->sem_magic != SEM_MAGIC) { //查看信号量结构体是否是经过初始化的
		errno = EINVAL;
		return(-1);
	}

/* *INDENT-OFF* */
	if (munmap(sem, sizeof(mysem_t)) == -1) //解除映射
		return(-1);
/* *INDENT-ON* */
	return(0);
}
/* end sem_close */

void
Mysem_close(mysem_t *sem)
{
	if (mysem_close(sem) == -1)
		err_sys("mysem_close error");
}

