/* include sem_post */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_post(mysem_t *sem)
{
	struct sembuf	op; //此结构体用来对信号量集中的信号量进行操作

	if (sem->sem_magic != SEM_MAGIC) { //检查sem_magic是否完成设置
		errno = EINVAL; //没完成设置
		return(-1); //返回-1
	}

	op.sem_num = 0; //信号量集中的第几个信号量
	op.sem_op = 1; //新信号量的操作为+1
	op.sem_flg = 0; //不设置nowait和undo标志位
	if (semop(sem->sem_semid, &op, 1) < 0) //用填写好的op结构体,对信号量集进行操作
		return(-1); //出错返回-1
	return(0); //正常返回0
}
/* end sem_post */

void
Mysem_post(mysem_t *sem)
{
	if (mysem_post(sem) == -1)
		err_sys("mysem_post error");
}
