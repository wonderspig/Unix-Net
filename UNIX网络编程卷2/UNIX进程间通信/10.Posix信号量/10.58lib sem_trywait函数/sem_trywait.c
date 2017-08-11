/* include sem_trywait */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_trywait(mysem_t *sem)
{
	struct sembuf	op; //操作信号量的结构体

	if (sem->sem_magic != SEM_MAGIC) { //检查sem_magic是否完成设置
		errno = EINVAL; //没有完成设置,就设置errno
		return(-1); //返回-1
	}

	op.sem_num = 0; //操作信号量集中的第一个信号量
	op.sem_op = -1; //对信号量进行-1操作
	op.sem_flg = IPC_NOWAIT; //设置nowait标志位
	if (semop(sem->sem_semid, &op, 1) < 0) //按照填写好的结构体,对信号量进行操作
		return(-1); //失败返回-1
	return(0); //成功返回0
}
/* end sem_trywait */

int
Mysem_trywait(mysem_t *sem)
{
	int		rc;

	if ( (rc = mysem_trywait(sem)) == -1 && errno != EAGAIN)
		err_sys("mysem_trywait error");
	return(rc);
}
