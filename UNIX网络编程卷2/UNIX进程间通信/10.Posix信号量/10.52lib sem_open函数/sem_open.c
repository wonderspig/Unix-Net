/* include sem_open1 */
#include	"unpipc.h"
#include	"semaphore.h"

#include	<stdarg.h>		/* for variable arg lists */
#define		MAX_TRIES	10	/* for waiting for initialization */

mysem_t	*
mysem_open(const char *pathname, int oflag, ... )
{
	int		i, fd, semflag, semid, save_errno;
	key_t	key;
	mode_t	mode;
	va_list	ap;
	mysem_t	*sem;
	union semun	arg;
	unsigned int	value;
	struct semid_ds	seminfo;
	struct sembuf	initop;

		/* 4no mode for sem_open() w/out O_CREAT; guess */
	semflag = 0644; //设置信号量的权限位
	semid = -1; //信号量的ID初始化为-1

	if (oflag & O_CREAT) { //如果传给mysem_open的参数要求创建信号量,则获取变长参数的数据(一共四个参数)
		va_start(ap, oflag);		/* init ap to final named argument 初始化ap,使ap指向变长参数前的最后一个参数 */
		//返回变长参数的第一个参数,ap向后移动va_mode_t的位置
		mode = va_arg(ap, va_mode_t); //va_mode_t在BSD系统上会被扩展为32位整数int,其他系统上为unsigned short
		value = va_arg(ap, unsigned int); //取ap指向的第二个参数,并让ap向后移动unsigned int的位置
		va_end(ap); //释放ap,结束可变参数的获取

			/* 4convert to key that will identify System V semaphore */
		if ( (fd = open(pathname, oflag, mode)) == -1) //open打开传参进来的路径,所要求的文件,oflag为权限位,mode为模式
			return(SEM_FAILED); //打开失败,返回SEM_FAILED
		close(fd); //关闭文件,打开依次不过使为了验证路径名所代表的文件是否有效
		if ( (key = ftok(pathname, 0)) == (key_t) -1) //把这个确定有效的路径名转换为创建System V信号量所需要的key
			return(SEM_FAILED); //失败返回SEM_FAILED

		semflag = IPC_CREAT | (mode & 0777); //设置信号量的权限位为合法值(兼容调用者没有指定mode的情况)
		if (oflag & O_EXCL) //在有O_CREAT标志的情况下,还有O_EXCL标志
			semflag |= IPC_EXCL; //加入IPC_EXCL标志

			/* 4create the System V semaphore with IPC_EXCL */
		//用准备好的权限位和准备好的key创建System V信号量(排他的创建信号量,是为了验证信号量是否存在)
		if ( (semid = semget(key, 1, semflag | IPC_EXCL)) >= 0) {
			//如果信号量不存在,就进入此分支进行创建信号量之后的避免竞争的初始化工作
			//先用semctl设置信号量的值
			//再使用semop对信号量进行第一次操作(此函数会设置sem_otime的值为当前值)
			//其他线程会检查这个值是否非0,来避免竞争状态

				/* 4success, we're the first so initialize to 0 */
			arg.val = 0; //设置信号量的值
			if (semctl(semid, 0, SETVAL, arg) == -1) //设置信号量
				goto err; //出错处理
				/* 4then increment by value to set sem_otime nonzero */
			if (value > SEMVMX) { //传参进来所要求的信号量的值,大于最大信号量的值
				errno = EINVAL; //设置errno
				goto err; //错误处理
			}
			//semop函数的参数
        	initop.sem_num = 0; //操作信号量集中的第一个信号量的值
        	initop.sem_op  = value; //对信号量的值进行操作
        	initop.sem_flg = 0; //标志,是非阻塞模式,还是UNDO模式
			if (semop(semid, &initop, 1) == -1) //按照initop结构体的内容对信号量集进行操作
				goto err; //错误处理
			goto finish; //完成处理

		} else if (errno != EEXIST || (semflag & IPC_EXCL) != 0)
		  //如果semget返回<=0,且调用者指定了IPC_EXCL标志,则出错
		  //如果调用者没有指定IPC_EXCL标志,则不出错,代码继续向后走,进入打开已存在信号量的部分
			goto err;
		/* else fall through */
	}
/* end sem_open1 */
/* include sem_open2 */
	/*
	 * (O_CREAT not secified) or
	 * (O_CREAT without O_EXCL and semaphore already exists).
	 * Must open semaphore and make certain it has been initialized.
	 */
	 //打开已存在的信号量

	if ( (key = ftok(pathname, 0)) == (key_t) -1) //用传参进来的路径名生成key
		goto err; //如果生成失败则报错
	if ( (semid = semget(key, 0, semflag)) == -1) //用指定路径名生成的key和semflag打开已存在的信号量
		goto err; //打开失败则报错

	arg.buf = &seminfo; //seminfo结构用来存放System V信号量的信息
	//MAX_TRIES为最大尝试次数
	//用此数进行循环,获得信号量的信息之后
	//查看sem_otime的值,sem_otime的值非0,才表示此信号量已经初始化完毕,可以使用semop进行操作
	for (i = 0; i < MAX_TRIES; i++) {
		if (semctl(semid, 0, IPC_STAT, arg) == -1) //获得信号量集中第一个信号量的信息,填入arg
			goto err; //失败则报错
		if (arg.buf->sem_otime != 0) //检查sem_otime的值是否为非0
			goto finish;
		sleep(1); //睡一秒,继续循环检查
	}
	errno = ETIMEDOUT; //超过最大的尝试次数之后,sem_otime仍然为0,则设置错误值
err:
	//错误处理
	save_errno = errno;		/* don't let semctl() change errno 临时保存错误值(防止下面的操作再次修改错误值)*/
	if (semid != -1) //调用semget失败而进入此代码段的情况
		semctl(semid, 0, IPC_RMID); //删除此信号量集中的第一个信号量
	errno = save_errno; //重新把备份的errno值设置回去
	return(SEM_FAILED); //返回错误码SEM_FAILED

finish:
	//完成工作后的处理
/* *INDENT-OFF* */
	if ( (sem = malloc(sizeof(mysem_t))) == NULL) //动态分配一个mysem_t的结构体指针
		goto err; //出错处理
/* *INDENT-ON* */
	sem->sem_semid = semid; //对mysem_t结构体进行填写设置
	sem->sem_magic = SEM_MAGIC; //设置宏SEM_MAGIC代表sem_open函数运行成功
	return(sem); //返回mysem_t结构体的指针
}
/* end sem_open2 */

//sem_open的包裹函数
mysem_t	*
Mysem_open(const char *pathname, int oflag, ... )
{
	va_list	ap;
	mode_t	mode;
	mysem_t	*sem;
	unsigned int	value;
	//如果指定了O_CREAT标志,则取变长参数之后，调用mysem_open
	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		value = va_arg(ap, unsigned int);
		if ( (sem = mysem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
		va_end(ap);
	} else { //如果没有指定O_CREAT标志,则直接用路径和oflag调用mysem_open
		if ( (sem = mysem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	}

	return(sem);
}
