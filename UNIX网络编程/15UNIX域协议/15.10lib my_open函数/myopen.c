#include	"unp.h"

int
my_open(const char *pathname, int mode)
{
	int			fd, sockfd[2], status;
	pid_t		childpid;
	char		c, argsockfd[10], argmode[10];

	Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd); //匿名uinx域套接字,socketfd中的两个套接字互相连接
	//创建一个子进程
	if ( (childpid = Fork()) == 0) {		/* child process 在子进程中*/
		Close(sockfd[0]); //关闭套接字的一端
		snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]); //储存套接字的描述符
		snprintf(argmode, sizeof(argmode), "%d", mode); //储存传参进来的模式
		//打开新的程序,把连接到父进程的套接字,需要打开的文件路径,打开文件的模式传递给它
		execl("./openfile", "openfile", argsockfd, pathname, argmode,
			  (char *) NULL); 
		err_sys("execl error");
	}

	/* parent process - wait for the child to terminate 在父进程中等待子进程终结 */
	Close(sockfd[1]);			/* close the end we don't use 关闭描述符的另一端*/

	Waitpid(childpid, &status, 0); //永久阻塞的等待子进程结束
	if (WIFEXITED(status) == 0) //查看终止的子进程状态,若等于0,则代表子进程非正常终止(被信号终止)
		err_quit("child did not terminate");
	if ( (status = WEXITSTATUS(status)) == 0) //提取子进程退出状态的状态值
		Read_fd(sockfd[0], &c, 1, &fd); //从子进程中读取文件描述符
	else {
		//代码走到这里代表子进程终止了，但是退出状态不为0

		/* set errno value from child's status 则保存子进程的错误状态, fd设置为-1，代表没有接收到描述符*/
		errno = status;		
		fd = -1;
	}

	Close(sockfd[0]); 
	return(fd);
}
