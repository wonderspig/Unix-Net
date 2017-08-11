/* include lock_reg */
#include	"unpipc.h"

int
lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock	lock;

	lock.l_type = type;		/* F_RDLCK, F_WRLCK, F_UNLCK 设置的锁的类型 */
	lock.l_start = offset;	/* byte offset, relative to l_whence 起始位置偏移量 */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END 起始位置 */
	lock.l_len = len;		/* #bytes (0 means to EOF) 锁住的文件长度 */

	return( fcntl(fd, cmd, &lock) );	/* -1 upon error 设置锁*/
}
/* end lock_reg */

void
Lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	if (lock_reg(fd, cmd, type, offset, whence, len) == -1)
		err_sys("lock_reg error");
}
