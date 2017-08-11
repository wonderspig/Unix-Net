#include	"unpthread.h"
#include	"pthread07.h"

void
thread_make(int i)
{
	void	*thread_main(void *);
	//创建线程,线程属性为null,线程id填写到线程结构体中,索引i按参数传递给线程
	Pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *) i); 
	return;		/* main thread returns */
}

void *
thread_main(void *arg)
{
	int				connfd;
	void			web_child(int);
	socklen_t		clilen;
	struct sockaddr	*cliaddr;

	cliaddr = Malloc(addrlen); //分配客户端地址结构体空间

	printf("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		clilen = addrlen;
    	Pthread_mutex_lock(&mlock);
		connfd = Accept(listenfd, cliaddr, &clilen); //在互斥锁的保护下accept
		Pthread_mutex_unlock(&mlock);
		tptr[(int) arg].thread_count++; //这个线程处理的客户数目递增1

		web_child(connfd);		/* process request 在此函数中响应客户的请求 */
		Close(connfd); //关闭连接到客户的套接字
	}
}
