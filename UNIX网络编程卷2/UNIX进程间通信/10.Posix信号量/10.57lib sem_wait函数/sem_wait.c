/* include sem_wait */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_wait(mysem_t *sem)
{
	struct sembuf	op; //操作信号量的结构体

	if (sem->sem_magic != SEM_MAGIC) { //检查sem_magic是否完成设置
		errno = EINVAL; //没有完成设置,就设置errno
		return(-1); //返回-1
	}

	op.sem_num = 0; //操作信号量集中的第一个信号量
	op.sem_op = -1; //对信号量进行-1操作
	op.sem_flg = 0; //不设置nowait和undo标志位
	if (semop(sem->sem_semid, &op, 1) < 0) //按照填写好的结构体,对信号量进行操作
		return(-1);
	return(0); //成功返回0
}
/* end sem_wait */

void
Mysem_wait(mysem_t *sem)
{
	if (mysem_wait(sem) == -1)
		err_sys("mysem_wait error");
}
