#include	"unpipc.h"

void
my_lock(int fd)
{
	struct flock	lock;

	lock.l_type = F_WRLCK; //类型为一把写锁
	lock.l_whence = SEEK_SET; //起始位置从文件开始处
	lock.l_start = 0; //起始位置偏移量为0
	lock.l_len = 0;				/* write lock entire file 锁定整个文件 */

	Fcntl(fd, F_SETLKW, &lock); //设置这把锁
}

void
my_unlock(int fd)
{
	struct flock	lock;

	lock.l_type = F_UNLCK; //类型为无锁
	lock.l_whence = SEEK_SET; //起始位置为文件开始处
	lock.l_start = 0; //起始位置偏移量
	lock.l_len = 0;				/* unlock entire file 解锁整个文件*/

	Fcntl(fd, F_SETLK, &lock); //设置这把锁
}
