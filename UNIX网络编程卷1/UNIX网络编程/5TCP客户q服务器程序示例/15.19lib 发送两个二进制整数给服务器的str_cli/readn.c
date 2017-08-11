/* include readn */
#include	"unp.h"

ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
	size_t	nleft; //剩余需要读的字节数
	ssize_t	nread; //已经读的字节数
	char	*ptr;  //读到的字节数存放的缓冲区

	ptr = vptr;
	nleft = n;
	while (nleft > 0) { //检查要读的字节数n的有效性
		if ( (nread = read(fd, ptr, nleft)) < 0) { //用循环从fd中读nleft个字节
			if (errno == EINTR) //被信号打断
				nread = 0;		/* and call read() again */
			else
				return(-1); //出错
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread; //剩余的字节数=现在剩余的字节数-已读的字节数
		ptr   += nread; //下一次要写入的prt指针的位置要右移已经读到的字节数
	}

	//已经读的字节数＝要求读的字节数-剩余的字节数
	return(n - nleft);		/* return >= 0 返回已经读的字节数*/
}
/* end readn */

ssize_t
Readn(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = readn(fd, ptr, nbytes)) < 0)
		err_sys("readn error");
	return(n);
}
