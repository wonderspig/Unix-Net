/* include child_make */
#include	"unp.h"
#include	"child.h"

pid_t
child_make(int i, int listenfd, int addrlen)
{
	int		sockfd[2];
	pid_t	pid;
	void	child_main(int, int, int);

	Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd); //创建一个匿名的全双工管道(父子进程之间使用)

	if ( (pid = Fork()) > 0) { //创建子进程
		//在父进程中
		Close(sockfd[1]); //关闭管道的一段
		cptr[i].child_pid = pid; //把子进程的pid填写到结构体中
		cptr[i].child_pipefd = sockfd[0]; //连接到这个子进程的全双工管道描述符
		cptr[i].child_status = 0; //这个子进程的状态(是否忙碌)
		//父进程返回,子进程不返回
		return(pid);		/* parent 父进程中返回子进程的pid */
	}
	//在子进程中
	Dup2(sockfd[1], STDERR_FILENO);		/* child's stream pipe to parent 复制连接到父进程的描述符到标准错误 */
	//把全双工管道的两端都关闭(一端无用,一端已经复制)
	Close(sockfd[0]); 
	Close(sockfd[1]);

	/* child does not need this open 关闭监听套接字(不再调用accept不需要监听套接字,连接搭配客户端的套接字会由父进程传送过来)*/
	Close(listenfd);					
	child_main(i, listenfd, addrlen);	/* never returns 调用这个函数完成请求 */
}
/* end child_make */

/* include child_main */
void
child_main(int i, int listenfd, int addrlen)
{
	char			c;
	int				connfd;
	ssize_t			n;
	void			web_child(int);

	printf("child %ld starting\n", (long) getpid());
	for ( ; ; ) {
		if ( (n = Read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0) //从父进程读取传送过来的文件描述符
			err_quit("read_fd returned 0");
		if (connfd < 0) //没有收到父进程传送过来的文件
			err_quit("no descriptor from read_fd");

		web_child(connfd);				/* process request 响应客户请求*/
		Close(connfd); //关闭连接到客户的文件描述符

		Write(STDERR_FILENO, "", 1);	/* tell parent we're ready again 向父进程写一个字节,说明这个子进程又处于空闲状态了 */
	}
}
/* end child_main */
