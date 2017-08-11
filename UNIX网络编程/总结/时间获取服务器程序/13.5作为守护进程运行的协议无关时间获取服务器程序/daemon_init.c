#include	"unp.h"
#include	<syslog.h>

#define	MAXFD	64

extern int	daemon_proc;	/* defined in error.c */

int
daemon_init(const char *pname, int facility)
{
	int		i;
	pid_t	pid;

	if ( (pid = Fork()) < 0) //创建一个子进程
		return (-1);
	else if (pid)
		_exit(0);			/* parent terminates 父进程退出*/

	//子进程继续运行
	//此进程是子进程,所以肯定不是组长进程

	/* child 1 continues... */

	if (setsid() < 0)			/* become session leader 创建会话,成为会话首进程,新的进程组的组长进程*/
		return (-1);

	Signal(SIGHUP, SIG_IGN); //把挂起信号设置为忽略
	if ( (pid = Fork()) < 0) //再创建一个子进程
		return (-1);
	else if (pid) //父进程退出
		_exit(0);			/* child 1 terminates */

	//第二个子进程继续运行,因为第二个子进程已经不是会话首进程了,所以永远不会获得控制终端

	/* child 2 continues... */

	daemon_proc = 1;			/* for err_XXX() functions 再error.c中定义的变量*/

	chdir("/");				/* change working directory 调整工作目录到根目录 */

	/* close off file descriptors */
	for (i = 0; i < MAXFD; i++) //关闭所有文件描述符
		close(i);

	/* redirect stdin, stdout, and stderr to /dev/null 定义标准输入,标准输出和标准错误到/dev/null */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	openlog(pname, LOG_PID, facility); //打开日志文件

	return (0);				/* success 函数运行成功 */
}
