/* include sem_open1 */
#include	"unpipc.h"
#include	"semaphore.h"

#include	<stdarg.h>		/* for variable arg lists */
#define		MAX_TRIES	10	/* for waiting for initialization */

mysem_t	*
mysem_open(const char *pathname, int oflag, ... )
{
	int		fd, i, created, save_errno;
	mode_t	mode;
	va_list	ap;
	mysem_t	*sem, seminit;
	struct stat	statbuff;
	unsigned int	value;
	pthread_mutexattr_t	mattr;
	pthread_condattr_t	cattr;

	created = 0; //标志:表示该信号量是打开还是创建(初始化为打开)
	sem = MAP_FAILED;				/* [sic] 信号量描述符初始化为MAP_FAILED*/
again:
	if (oflag & O_CREAT) { //如果调用者提供的参数中有O_CREAT标志(那就说明需要变长参数)
		va_start(ap, oflag);		/* init ap to final named argument 初始化ap结构体,获得变长参数*/
		//获得第一个变长参数(文件的权限位),并关闭可执行位
		//可变参数的第一个参数为创建消息队列的权限位,我们去除消息队列的"用户可执行位"
		//因为创建一个消息队列(只不过创建一个文件用来用作mmap的映射而已)和初始化一个消息队列这两个动作并不是原子进行的
		//所以在创建一个消息队列之后,打开其用户执行位
		//表示这个消息队列还没有初始化,在初始化消息队列之后,再关闭其用户执行位,表示这个消息队列初始化好了。
		//所以,在此地,我们预先关闭第三个参数的用户执行位,以免产生语义上得分歧
		mode = va_arg(ap, va_mode_t) & ~S_IXUSR; //不管系统是否把mode_t扩展为32位整型,为了可移植性,用va_mode_t代替
		value = va_arg(ap, unsigned int); //获得第二个变长参数,指针向后移动unsigned int
		va_end(ap); //变长参数全部取得,把ap结构体销毁

			/* 4open and specify O_EXCL and user-execute */

		//打开调用者所要求的路径,以调用者指定的模式,外加读写,排他(用来验证文件是否由本线程创建)模式
		//再加上调用者附上的权限位外加执行权限位(表示消息队列还没有初始化)来打开一个文件(该文件用来mmap映射)

		//调用者在oflag中可能指定O_CREAT,O_CREAT|O_EXCL,没有指定O_CREAT,O_EXCL这两个标志中的任何一个三种情况
		//如果调用者在oflag中只指定了O_CREAT:语义为如果消息队列不存在则创建,如果存在则打开这个消息队列
		//如果调用者在oflag中只指定了O_CREAT|O_EXCL,那么调用者必须创建一个消息队列,如存在则返回错误
		//如果调用者没有指定O_CREAT,O_EXCL这两个标志中的任何一个,则调用者不创建消息队列,只是单纯的打开一个消息队列
		//所以,为了符合这三种语义,在open返回EEXIST排他性错误时,我们检测oflag中是否开启O_EXCL位,如果oflag中开启了O_EXCL位
		//则说明调用者希望自己创建一个消息队列,现在文件已存在,我们返回调用者-1,表示错误
		//如果调用者的oflag中没有指定O_EXCL位,那就跳到exists去直接打开这个消息队列,就当O_CREAT位从来没有指定过
		fd = open(pathname, oflag | O_EXCL | O_RDWR, mode | S_IXUSR); 
		if (fd < 0) { //打开失败
			if (errno == EEXIST && (oflag & O_EXCL) == 0) 
				goto exists;		/* already exists, OK */
			else
				return(SEM_FAILED);
		}
		created = 1; //标志置为1,说明要创建一个新的信号量
			/* 4first one to create the file initializes it */
			/* 4set the file size */
		bzero(&seminit, sizeof(seminit)); //清空一个信号量结构体
		if (write(fd, &seminit, sizeof(seminit)) != sizeof(seminit)) //新创建的文件大小为0,用write来是文件的大小为sizeof(seminit)
			goto err;

			/* 4memory map the file */
		sem = mmap(NULL, sizeof(mysem_t), PROT_READ | PROT_WRITE,
				   MAP_SHARED, fd, 0); //映射这个新创建的文件
		if (sem == MAP_FAILED) //表示mmap失败
			goto err;

			/* 4initialize mutex, condition variable, and value 初始化互斥量和条件变量 */
		if ( (i = pthread_mutexattr_init(&mattr)) != 0) //初始化互斥量属性结构体
			goto pthreaderr;
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); //加入进程间共享属性
		i = pthread_mutex_init(&sem->sem_mutex, &mattr); //根据这个新属性初始化互斥量
		pthread_mutexattr_destroy(&mattr);	/* be sure to destroy 销毁互斥量属性结构体 */
		if (i != 0)
			goto pthreaderr; //互斥量初始化失败

		if ( (i = pthread_condattr_init(&cattr)) != 0) //初始化条件变量属性结构体
			goto pthreaderr;
		pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED); //为条件变量属性增加进程间共享属性
		i = pthread_cond_init(&sem->sem_cond, &cattr); //用这个属性初始化条件变量
		pthread_condattr_destroy(&cattr);	/* be sure to destroy 销毁这个条件变量属性 */
		if (i != 0) //条件变量初始化失败
			goto pthreaderr;

		if ( (sem->sem_count = value) > sysconf(_SC_SEM_VALUE_MAX)) { //如果调用者传递的信号量的初始值小于系统的限制值,则填写入信号量结构体
			errno = EINVAL;
			goto err;
		}
			/* 4initialization complete, turn off user-execute bit */
		if (fchmod(fd, mode) == -1) //关掉用户执行位,表示已经初始化完毕
			goto err;
		close(fd);
		sem->sem_magic = SEM_MAGIC; //设置标志,表示已经初始化完毕
		return(sem); //返回这个信号量结构体
	}
/* end sem_open1 */
/* include sem_open2 */
exists: //如果文件已存在(调用者定义了O_CREAT,没有定义O_EXCL,且文件确实已存在)
	if ( (fd = open(pathname, O_RDWR)) < 0) { //不创建了,直接读写方式打开文件
		//如果文件路径不存在且指定了O_CREAT标志(两个线程同时创建文件的情况下,会发生竞争状态,产生这个结果)
		if (errno == ENOENT && (oflag & O_CREAT)) 
			goto again;
		goto err; //其他错误不可接受
	}

	sem = mmap(NULL, sizeof(mysem_t), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //映射这个已存在的文件,读写模式
	if (sem == MAP_FAILED) //说明mmap映射失败
		goto err;

		/* 4make certain initialization is complete */
	//检查消息队列是否被初始化,检查的次数为MAX_TRIES
	for (i = 0; i < MAX_TRIES; i++) { 
		if (stat(pathname, &statbuff) == -1) { //获取文件属性
			//如果文件路径不存在且指定了O_CREAT标志(两个线程同时创建文件的情况下,会发生竞争状态,产生这个结果)
			if (errno == ENOENT && (oflag & O_CREAT)) { 
				close(fd); 
				goto again;
			}
			goto err;
		}
		if ((statbuff.st_mode & S_IXUSR) == 0) { //查看是否有执行位,没有执行位说明已经初始化完成
			close(fd); //关掉这个文件
			sem->sem_magic = SEM_MAGIC; //设置标志
			return(sem); //返回这个结构体给调用者
		}
		sleep(1); //睡一秒后继续尝试
	}
	errno = ETIMEDOUT; //检查的次数已过,但是还是没有检查到执行位
	goto err;

pthreaderr:
	errno = i;
err:
		/* 4don't let munmap() or close() change errno */
		//出错处理要小心的保存errno的值,防止再做清理工作的时候更改errno的值
	save_errno = errno;
	if (created)
		unlink(pathname);
	if (sem != MAP_FAILED)
		munmap(sem, sizeof(mysem_t));
	close(fd);
	errno = save_errno;
	return(SEM_FAILED);
}
/* end sem_open2 */

mysem_t	*
Mysem_open(const char *pathname, int oflag, ... )
{
	va_list	ap;
	mode_t	mode;
	mysem_t	*sem;
	unsigned int	value;

	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		value = va_arg(ap, unsigned int);
		va_end(ap);
		if ( (sem = mysem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	} else {
		if ( (sem = mysem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	}

	return(sem);
}
