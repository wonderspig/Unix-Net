/* include lock_test */
#include	"unpipc.h"

pid_t
lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock	lock;

	lock.l_type = type;		/* F_RDLCK or F_WRLCK 测试的锁的类型*/
	lock.l_start = offset;	/* byte offset, relative to l_whence 起始位置偏移量 */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END 起始位置*/
	lock.l_len = len;		/* #bytes (0 means to EOF) 锁定文件的长度*/

	if (fcntl(fd, F_GETLK, &lock) == -1) //获得锁的信息,填入lock结构体
		return(-1);			/* unexpected error */

	if (lock.l_type == F_UNLCK) //如果fd描述符所代表的文件没有上锁,则函数返回0
		return(0);			/* false, region not locked by another proc */
	return(lock.l_pid);		/* true, return positive PID of lock owner 否则返回占有锁的进程pid*/
}
/* end lock_test */

pid_t
Lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
	pid_t	pid;

	if ( (pid = lock_test(fd, type, offset, whence, len)) == -1)
		err_sys("lock_test error");
	return(pid);
}
