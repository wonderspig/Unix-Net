/* include sem_post */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_post(mysem_t *sem)
{
	char	c;

	if (sem->sem_magic != SEM_MAGIC) { //检查信号量结构提标志,验证是否初始化
		errno = EINVAL;
		return(-1);
	}

	if (write(sem->sem_fd[1], &c, 1) == 1) //往FIFO中写入一个字节(FIFO中的字节数就是信号量的值)
		return(0);
	return(-1);
}
/* end sem_post */

void
Mysem_post(mysem_t *sem)
{
	if (mysem_post(sem) == -1)
		err_sys("mysem_post error");
}
