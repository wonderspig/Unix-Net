#include	"unpipc.h"

#define	LOCKFILE	"/tmp/seqno.lock"

void
my_lock(int fd)
{
	int		tempfd;

	while ( (tempfd = open(LOCKFILE, O_RDWR|O_CREAT|O_EXCL, FILE_MODE)) < 0) { //以O_RDWR|O_CREAT|O_EXCL来打开文件
		if (errno != EEXIST) //不是EEXIST错误,则无法接受,退出！否则继续调用open
			err_sys("open error for lock file");
		/* someone else has the lock, loop around and try again */
	}
	Close(tempfd);			/* opened the file, we have the lock 关闭文件,文件本身就表示锁,与是否打开该文件无关 */
}

void
my_unlock(int fd)
{
	Unlink(LOCKFILE);		/* release lock by removing file 删除这个文件,就表示解锁 */
}
