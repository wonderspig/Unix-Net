/* include sem_open */
#include	"unpipc.h"
#include	"semaphore.h"

#include	<stdarg.h>		/* for variable arg lists */

mysem_t	*
mysem_open(const char *pathname, int oflag, ... )
{
	int		i, flags, save_errno;
	char	c;
	mode_t	mode;
	va_list	ap;
	mysem_t	*sem;
	unsigned int	value;

	if (oflag & O_CREAT) { //如果调用者指定的模式有O_CREAT,说明要有四个参数
		va_start(ap, oflag);		/* init ap to final named argument 初始化ap结构体,获得后面两个参数*/
		//为了可移植性,不管编译器有没有把mode_t扩展成32位
		mode = va_arg(ap, va_mode_t); //取得第一个变长参数,并把指针后移va_mode_t来指向第二个变长参数(创建FIFO的权限位,信号量的权限位)
		value = va_arg(ap, unsigned int); //取得第二个变长参数,且指针后移unsigned int(信号量的初始化值)
		va_end(ap); //销毁ap结构体

		if (mkfifo(pathname, mode) < 0) { //创建FIFO
			if (errno == EEXIST && (oflag & O_EXCL) == 0) //如果FIFO已存在,且调用者指定了O_EXCL,则报错返回
				oflag &= ~O_CREAT;	/* already exists, OK 否则去掉O_CREAT,单纯打开FIFO(防止后面再初始化这个FIFO)*/
			else
				return(SEM_FAILED); //失败返回
		}
	}

/* *INDENT-OFF* */
	if ( (sem = malloc(sizeof(mysem_t))) == NULL) //动态分配创建信号量描述符结构体
		return(SEM_FAILED); //动态分配失败
	sem->sem_fd[0] = sem->sem_fd[1] = -1; //用来读和用来写的描述符先初始化为-1

	if ( (sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0) //只读打开FIFO,存入sem->sem_fd[0]
		goto error;
	if ( (sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0) //只写打开FIFO,存入sem->sem_fd[1]
		goto error;
/* *INDENT-ON* */

		/* 4turn off nonblocking for sem_fd[0] */
	if ( (flags = fcntl(sem->sem_fd[0], F_GETFL, 0)) < 0) //获得只读描述符的权限位(为了去掉其中的非阻塞标志)
		goto error;
	flags &= ~O_NONBLOCK; //去掉只读权限位的非阻塞标志
	if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0) //把标志设置进只读FIFO描述符
		goto error;

	if (oflag & O_CREAT) {		/* initialize semaphore 标志中还有创建(代表这个FIFO是这次新创建的,而不是打开的)*/
		//初始化这个FIFO
		for (i = 0; i < value; i++) //初始化这个FIFO中的值(信号量的初始值是多少,就往FIFO中写入多少个字节)
			if (write(sem->sem_fd[1], &c, 1) != 1) //循环写入value个字节
				goto error;
	}

	sem->sem_magic = SEM_MAGIC; //初始化完毕,更改标志
	return(sem); //返回信号量描述符

error:
	save_errno = errno; //备份errno值(防止关闭信号量过程中更改errno的值)
	if (oflag & O_CREAT) //信号量是创建的
		unlink(pathname);		/* if we created FIFO 删除这个信号量*/
	close(sem->sem_fd[0]);		/* ignore error 关闭读FIFO*/
	close(sem->sem_fd[1]);		/* ignore error 关闭写FIFO*/
	free(sem);
	errno = save_errno; //恢复errno的值
	return(SEM_FAILED); //返回失败
}
/* end sem_open */

mysem_t	*
Mysem_open(const char *pathname, int oflag, ... )
{
	va_list	ap;
	mode_t	mode;
	mysem_t	*sem;
	unsigned int	value;

	if (oflag & O_CREAT) { //有O_CREAT就需要取得后面两个变长参数
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		value = va_arg(ap, unsigned int);
		if ( (sem = mysem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
		va_end(ap);
	} else { //否则直接使用前两个参数调用mysem_open
		if ( (sem = mysem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	}

	return(sem);
}
