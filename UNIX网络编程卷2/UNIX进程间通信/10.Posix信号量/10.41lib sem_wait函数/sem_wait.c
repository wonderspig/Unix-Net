/* include sem_wait */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_wait(mysem_t *sem)
{
	char	c;

	if (sem->sem_magic != SEM_MAGIC) { //检查信号量结构提标志,验证是否初始化
		errno = EINVAL;
		return(-1);
	}

	if (read(sem->sem_fd[0], &c, 1) == 1) //从FIFO读出一个数据(FIFO中数据的数量,就是信号量的值)
		return(0);
	return(-1);
}
/* end sem_wait */

void
Mysem_wait(mysem_t *sem)
{
	if (mysem_wait(sem) == -1)
		err_sys("mysem_wait error");
}
