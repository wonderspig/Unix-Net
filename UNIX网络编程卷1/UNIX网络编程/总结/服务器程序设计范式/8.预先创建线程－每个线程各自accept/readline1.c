/* include readline */
#include	"unp.h"

/* PAINFULLY SLOW VERSION -- example only */
ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr; //把传参进来的缓冲区复制一份
	for (n = 1; n < maxlen; n++) { //在一行的最大字节数maxlen之内循环
again:
		if ( (rc = read(fd, &c, 1)) == 1) { //每次读一个字节
			*ptr++ = c; //把读到的一个字节放入指针ptr指向的内存中,然后ptr右移一个单位
			if (c == '\n') //如果读到的是'\n',说明一行结束
				break;	/* newline is stored, like fgets() 一行已经存储完毕,跳出循环,就像fgets一样*/
		} else if (rc == 0) { //读到EOF(输入中没有'\n',就结束了)
			*ptr = 0; //让ptr字符串以"\0"结尾
			/* EOF, n - 1 bytes were read 返回已经读的字节数,这一次读到的是EOF,所以这次循环的n++要去除,减回来 */
			return(n - 1);	
		} else {
			if (errno == EINTR) //如果是被信号打断
				goto again; //重新调用read去读
			return(-1);		/* error, errno set by read() read读取错误,返回-1 */
		}
	}

	//代码走到这里,说明读到的数据中,是以'\n'来结束的一行
	*ptr = 0;	/* null terminate like fgets() 末尾加上'\0'字符 */
	return(n); //返回已经读的字节数(没有读到过EOF，每一个字节都要计算在内)
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}
