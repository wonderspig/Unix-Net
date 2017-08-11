#include	"unpipc.h"
#include	"pthread_rwlock.h"

pthread_rwlock_t	rwlock = PTHREAD_RWLOCK_INITIALIZER;

void	 *thread1(void *), *thread2(void *);
pthread_t	tid1, tid2;

int
main(int argc, char **argv)
{
	void	*status;
	Pthread_rwlock_init(&rwlock, NULL); //初始化读写锁

	Set_concurrency(2); //最多同时并发两个线程(linux不需要)
	Pthread_create(&tid1, NULL, thread1, NULL); //创建第一个线程,线程函数thread1
	sleep(1);		/* let thread1() get the lock 睡一秒让线程1获得一个读出锁*/
	Pthread_create(&tid2, NULL, thread2, NULL); //创建第二个线程,线程函数thread2

	Pthread_join(tid2, &status); //等待线程2结束,并把线程2的结束状态保存在status中
	if (status != PTHREAD_CANCELED) //如果线程的退出状态为PTHREAD_CANCELED
		printf("thread2 status = %p\n", status); 
	Pthread_join(tid1, &status); //等待线程1接收,并把线程1的结束状态保存在status中
	if (status != NULL) //如果线程的退出状态为NULL
		printf("thread1 status = %p\n", status);

	printf("rw_refcount = %d, rw_nwaitreaders = %d, rw_nwaitwriters = %d\n",
		   rwlock.rw_refcount, rwlock.rw_nwaitreaders,
		   rwlock.rw_nwaitwriters); //打印读写锁计数器中的值
	Pthread_rwlock_destroy(&rwlock);
		/* 4returns EBUSY error if cancelled thread does not cleanup */

	exit(0);
}

void *
thread1(void *arg)
{
	Pthread_rwlock_rdlock(&rwlock); //加读锁
	printf("thread1() got a read lock\n"); //打印获得了一把读锁
	sleep(3);		/* let thread2 block in pthread_rwlock_wrlock() 睡3秒,让线程2阻塞在等待加写锁的条件变量中 */
	pthread_cancel(tid2); //取消线程2,此时线程2在条件变量的等待中，直接被取消
	sleep(3); //再等待3秒
	Pthread_rwlock_unlock(&rwlock); //释放读锁退出
	return(NULL);
}

void *
thread2(void *arg)
{
	printf("thread2() trying to obtain a write lock\n"); //尝试获得一把写锁
	Pthread_rwlock_wrlock(&rwlock); //加写锁
	printf("thread2() got a write lock\n"); //获得了一把写锁
	sleep(1); //睡一秒
	Pthread_rwlock_unlock(&rwlock); //释放写锁退出
	return(NULL);
}
