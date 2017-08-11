#include	"unpipc.h"

#define	LOCK_PATH	"pxsemlock"

sem_t	*locksem;
int		initflag;

void
my_lock(int fd)
{
	if (initflag == 0) { //标志为0表示信号量没有打开
		locksem = Sem_open(Px_ipc_name(LOCK_PATH), O_CREAT, FILE_MODE, 1); //打开信号量(二值信号量)
		initflag = 1; //把标志置为1
	}
	Sem_wait(locksem); //使用一个信号量
}

void
my_unlock(int fd)
{
	Sem_post(locksem); //挂出一个信号量
}
