/* include sem_getvalue */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_getvalue(mysem_t *sem, int *pvalue)
{
	int		val; //保存信号量当前的值

	if (sem->sem_magic != SEM_MAGIC) { //检查sem_magic是否完成设置
		errno = EINVAL; //没有完成设置,就设置errno
		return(-1); //返回-1
	}

	if ( (val = semctl(sem->sem_semid, 0, GETVAL)) < 0) //获得信号量集中第一个信号量的值,填写在val中
		return(-1); //失败返回-1
	*pvalue = val; //把值放入调用者传参进来的指针所指向的内存中
	return(0); //返回0,表示调用成功
}
/* end sem_getvalue */

void
Mysem_getvalue(mysem_t *sem, int *pvalue)
{
	if (mysem_getvalue(sem, pvalue) == -1)
		err_sys("mysem_getvalue error");
}

