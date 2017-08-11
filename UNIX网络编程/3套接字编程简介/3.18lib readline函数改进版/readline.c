/* include readline */
#include	"unp.h"

static int	read_cnt; //维护当前缓冲区中的字节数
static char	*read_ptr; //这个缓冲区用来把read读出来的数据每次一个的压入参数ptr中
static char	read_buf[MAXLINE]; //这个缓冲区用来存放read读出来的数据

static ssize_t
my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) { //当前缓冲区中没有数据
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) { //一次读取MAXLINE个字节
			if (errno == EINTR) //如果read被信号打断,则重新调用read
				goto again;
			return(-1); //read出错,返回-1
		} else if (read_cnt == 0) //已经读到EOF
			return(0); //返回0
		read_ptr = read_buf;
	}

	//已经一次读入了MAXLINE个字节,实际读入read_cnt个字节
	//下面每次返回1个字节给调用者,如果read_cnt个字节都返回完了,会继续用read去读

	read_cnt--; //缓冲区中剩余的字节数-1
	*ptr = *read_ptr++; //返回一个字节,并且把指针右移一个单位
	return(1); //返回1(读到一个字节给调用者)
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr; //把传参进来的缓冲区复制一份
	for (n = 1; n < maxlen; n++) { //在一行允许的最大字节数内循环
		if ( (rc = my_read(fd, &c)) == 1) { //一次读入一个字节(自己维护缓冲区)
			*ptr++ = c; //把读到的字节放入ptr中,且ptr右移一个单位
			if (c == '\n') //如果读到是'\n',说明一行结束,跳出循环,就好像fgets一样
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) { //读到EOF
			*ptr = 0; //缓冲区以'\0'结尾
			return(n - 1);	/* EOF, n - 1 bytes were read 返回读到的字节数,因为这次n++读到的是EOF,所以n－1 */
		} else
			return(-1);		/* error, errno set by read() 出错返回-1 */
	}

	*ptr = 0;	/* null terminate like fgets() 在文件尾端加上'\0' */
	return(n); //返回已经读到的字节数,因为代码走到这里是以\n结束,所以不在需要n-1
}

ssize_t
readlinebuf(void **vptrptr)
{
	if (read_cnt) //查看缓冲区中是否有数据
		*vptrptr = read_ptr; //有数据，就浅复制缓冲区数据到参数vptrptr中
	return(read_cnt); //并返回缓冲区中的当前字节数
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
