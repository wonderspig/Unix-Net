/* include sem_close */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_close(mysem_t *sem)
{
	if (sem->sem_magic != SEM_MAGIC) { //检查sem_magic,看看sem_open是否成功运行
		errno = EINVAL; //sem_open没成功则出错
		return(-1); //返回-1
	}
	sem->sem_magic = 0;		/* just in case 设置sem_magic为0*/

	free(sem); //释放sem的资源
	return(0);
}
/* end sem_close */

void
Mysem_close(mysem_t *sem)
{
	if (mysem_close(sem) == -1)
		err_sys("mysem_close error");
}

