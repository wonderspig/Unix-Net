#include	"unpthread.h"
#include	"pthread08.h"

void
thread_make(int i)
{
	void	*thread_main(void *);
	int     *val;
	val = Malloc(sizeof(int));
	*val = i;
	//创建线程,线程属性为默认,把线程信息结构体数组的下标作为参数传递给线程
	Pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *) val); 
	return;		/* main thread returns */
}

void *
thread_main(void *arg)
{
	int		connfd;
	void	web_child(int);

	printf("thread %d starting\n", *((int*)arg));
	free(arg);
	
	for ( ; ; ) {
    	Pthread_mutex_lock(&clifd_mutex); //互斥量加锁
    	//iput是主线程将往该数组中存入的下一个元素的下标,iget是线程池中某个线程从该数组中取出的下一个元素的下标
    	//如果这两者相等代表还没有新的连接到客户端的文件描述符放入线程信息结构体数组
		while (iget == iput)
			Pthread_cond_wait(&clifd_cond, &clifd_mutex); //休眠等待条件变量
		connfd = clifd[iget];	/* connected socket to service 从数组中取出一个文件描述符 */
		//iget是线程池中某个线程从该数组中取出的下一个元素的下标,该下标往后走一格,如果超过数组边界,则回绕到0
		if (++iget == MAXNCLI)
			iget = 0;
		Pthread_mutex_unlock(&clifd_mutex); //释放互斥锁
		//虽然是在操作全局变量,但是全局变量的这个成员并没有其他线程会访问,所以不加锁也是安全的
		tptr[(int) arg].thread_count++; //本线程处理的客户数目+1

		web_child(connfd);		/* process request 在这个函数中,响应客户的请求*/
		Close(connfd); //关闭连接到客户的套接字
	}
}
