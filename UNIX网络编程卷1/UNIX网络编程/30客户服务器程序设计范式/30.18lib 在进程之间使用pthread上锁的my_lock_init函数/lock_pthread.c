/* include my_lock_init */
#include	"unpthread.h"
#include	<sys/mman.h>

static pthread_mutex_t	*mptr;	/* actual mutex will be in shared memory */

void
my_lock_init(char *pathname)
{
	int		fd;
	pthread_mutexattr_t	mattr; //线程互斥量属性

	fd = Open("/dev/zero", O_RDWR, 0); //打开设备"/dev/zero",用来创建匿名共享存储区

	mptr = Mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0); //创建共享存储区
	Close(fd); //关闭设备"/dev/zero"

	Pthread_mutexattr_init(&mattr); //初始化互斥量属性
	Pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); //为互斥量属性加入PTHREAD_PROCESS_SHARED,使互斥量可以在进程之间使用
	Pthread_mutex_init(mptr, &mattr); //初始化互斥量
}
/* end my_lock_init */

/* include my_lock_wait */
void
my_lock_wait()
{
	Pthread_mutex_lock(mptr); //加锁互斥量
}

void
my_lock_release()
{
	Pthread_mutex_unlock(mptr); //解锁互斥量
}
/* end my_lock_wait */
