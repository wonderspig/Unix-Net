/* include child_make */
#include	"unp.h"

pid_t
child_make(int i, int listenfd, int addrlen)
{
	pid_t	pid;
	void	child_main(int, int, int);

	if ( (pid = Fork()) > 0) //创建子进程(并且在父进程中)
		return(pid);		/* parent 在父进程中,像本函数调用者返回子进程的pid */

	child_main(i, listenfd, addrlen);	/* never returns 在子进程中,调用这个函数进行处理 */
}
/* end child_make */

/* include child_main */
void
child_main(int i, int listenfd, int addrlen)
{
	int				connfd;
	void			web_child(int);
	socklen_t		clilen;
	struct sockaddr	*cliaddr;
	void            my_lock_wait();
	void            my_lock_release();

	cliaddr = Malloc(addrlen); //分配存放客户地址的地址结构体空间

	printf("child %ld starting\n", (long) getpid());
	for ( ; ; ) {
		clilen = addrlen;
		my_lock_wait();
		connfd = Accept(listenfd, cliaddr, &clilen); //产生连接到客户的套接字
		my_lock_release();

		web_child(connfd);		/* process the request 响应客户的请求*/
		Close(connfd); //关闭连接到客户的套接字
	}
}
/* end child_main */
