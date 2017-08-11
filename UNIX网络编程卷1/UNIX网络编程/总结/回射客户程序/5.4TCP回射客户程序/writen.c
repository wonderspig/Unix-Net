/* include writen */
#include	"unp.h"

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft; //剩余要写的字节数
	ssize_t		nwritten; //已经写的字节数
	const char	*ptr; //write的缓冲区

	ptr = vptr; //把传参进来的write要写的缓冲区备份一份
	nleft = n; //还剩余需要写的字节数初始化为总共需要写的字节数
	while (nleft > 0) { //检查传参进来的需要写的字节数的有效性
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) { //把ptr写入fd
			if (nwritten < 0 && errno == EINTR) //当write返回值小于0且因为是被信号打断
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error 其他小于0的情况为错误*/
		}

		nleft -= nwritten; //还剩余需要写的字节数=现在还剩余需要写的字节数-这次已经写的字节数
		ptr   += nwritten; //下次开始写的缓冲区位置=缓冲区现在的位置右移已经写了的字节数大小
	}
	return(n); //返回已经写了的字节数
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}
