/* include sem_unlink */
#include	"unpipc.h"
#include	"semaphore.h"

int
mysem_unlink(const char *pathname)
{
	if (unlink(pathname) == -1) //删除信号量文件
		return(-1);
	return(0);
}
/* end sem_unlink */

void
Mysem_unlink(const char *pathname)
{
	if (mysem_unlink(pathname) == -1)
		err_sys("mysem_unlink error");
}

