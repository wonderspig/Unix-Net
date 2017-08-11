/* include sem_close */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_close(mysem_t *sem)
{
	if (sem->sem_magic != SEM_MAGIC) { //检查信号量描述符结构体是否被初始化过
		errno = EINVAL;
		return(-1);
	}

	sem->sem_magic = 0;		/* in case caller tries to use it later 把标志艾薇没有初始化过*/
	if (close(sem->sem_fd[0]) == -1 || close(sem->sem_fd[1]) == -1) { //把FIFO的读描述符和写描述符都关闭
		//其中一个FIFO关闭失败
		free(sem); 
		return(-1);
	}
	free(sem); //释放信号量描述符结构体
	return(0);
}
/* end sem_close */

void
Mysem_close(mysem_t *sem)
{
	if (mysem_close(sem) == -1)
		err_sys("mysem_close error");
}

