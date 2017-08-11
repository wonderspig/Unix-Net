/* include sem_unlink */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_unlink(const char *pathname)
{
	int		semid;
	key_t	key;

	if ( (key = ftok(pathname, 0)) == (key_t) -1) //通过pathname生成key
		return(-1); //出错返回-1
	if (unlink(pathname) == -1) //删除pathname所指向的文件
		return(-1); //出错返回-1
	if ( (semid = semget(key, 1, 0644)) == -1) //打开key所代表的消息队列
		return(-1); //出错返回-1
	if (semctl(semid, 0, IPC_RMID) == -1) //删除信号量集semid中的第一个信号量
		return(-1); //出错返回-1
	return(0);
}
/* end sem_unlink */

void
Mysem_unlink(const char *pathname)
{
	if (mysem_unlink(pathname) == -1)
		err_sys("mysem_unlink error");
}

