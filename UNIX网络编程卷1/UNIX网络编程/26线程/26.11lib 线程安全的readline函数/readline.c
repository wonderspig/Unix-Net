/* include readline1 */
#include	"unpthread.h"

static pthread_key_t	rl_key;
static pthread_once_t	rl_once = PTHREAD_ONCE_INIT;
//线程特定数据的析构函数
static void
readline_destructor(void *ptr)
{
	free(ptr);
}
//Pthread_once需要的函数,用来调用Pthread_key_create
static void
readline_once(void)
{
	Pthread_key_create(&rl_key, readline_destructor);
}

typedef struct {
  int	 rl_cnt;			/* initialize to 0 初始化为0,已经读取的字节数 */
  char	*rl_bufptr;			/* initialize to rl_buf 初始化为指向rl_buf数组*/
  char	 rl_buf[MAXLINE];   /* rl_buf数组:存放一下子读进来的数据(readline函数在从这个缓冲区中一个个字节的读) */
} Rline;
/* end readline1 */

/* include readline2 */
static ssize_t
my_read(Rline *tsd, int fd, char *ptr)
{
	if (tsd->rl_cnt <= 0) {
again:
		//从套接字中读取数据,读到的数据放在tsd->rl_buf缓冲区中
		if ( (tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
			if (errno == EINTR) //被信号打断则返回继续读
				goto again;
			return(-1); //别的错误就返回-1
		} else if (tsd->rl_cnt == 0) //读到EOF的情况下,返回0
			return(0);
		tsd->rl_bufptr = tsd->rl_buf; //给指针tsd->rl_bufptr赋值,让它指向read存放数据的缓冲区
	}

	tsd->rl_cnt--; //给调用者返回一个字节数据
	*ptr = *tsd->rl_bufptr++; //把指针当前指向的数据返回给调用者,且指针往后走一格
	return(1); //总归只返回一格字节
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	size_t		n, rc;
	char	c, *ptr;
	Rline	*tsd;
	//保证readline_once函数只调用一次,readline_once函数中创建一个键(关联到线程特定数据Rline)
	Pthread_once(&rl_once, readline_once); 
	//获得线程特定数据,如果没有线程特定数据,说明是第一次调用readline函数
	//则动态分配一个内存空间(Rline的大小,并全部初始化为0)
	if ( (tsd = pthread_getspecific(rl_key)) == NULL) { 
		tsd = Calloc(1, sizeof(Rline));		/* init to 0 */
		Pthread_setspecific(rl_key, tsd); //把这个key关联到内存特定数据(tsd－Rline)
	}

	ptr = vptr; //把传参进来的,用来存放一整行数据的缓冲区指针,浅复制一份
	for (n = 1; n < maxlen; n++) { //用循环一次次的从my_read中,每次拿出一个字节,拼出一行
		if ( (rc = my_read(tsd, fd, &c)) == 1) { //my_read从它自己的缓冲区中,每次返回给调用者一个字节
			*ptr++ = c; //把my_read取得的字符串c放入ptr指向的内存中,ptr往后走一格,用来存放下一个字符
			if (c == '\n') //如果这次读到的是换行符(说明一行已经结束)
				break; //跳出循环
		} else if (rc == 0) { //如果读到EOF,说明已经结束输入
			*ptr = 0; //当前ptr指向的字符为0,末尾NULL
			return(n - 1);		/* EOF, n - 1 bytes read 返回读到的字节数,因为这次n++读到的是EOF,所以n－1*/
		} else
			return(-1);		/* error, errno set by read() 其他错误返回-1*/
	}

	*ptr = 0; //加上末尾的NULL字符
	return(n); //返回已经读到的字节数,因为代码走到这里是以\n结束,所以不在需要n-1
}
/* end readline2 */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}
