#include	"unpipc.h"

#define	PATH_PIDFILE	"pidfile"

int
main(int argc, char **argv)
{
	int		pidfd;
	char	line[MAXLINE];

		/* 4open the PID file, create if nonexistent */
	pidfd = Open(PATH_PIDFILE, O_RDWR | O_CREAT, FILE_MODE); //读写创建的方式打开一个文件

		/* 4try to write lock the entire file */
	if (write_lock(pidfd, 0, SEEK_SET, 0) < 0) { //获得一把写锁
		if (errno == EACCES || errno == EAGAIN) //无法获取写锁,已经有一个守护进程副本在运行
			err_quit("unable to lock %s, is %s already running?",
					 PATH_PIDFILE, argv[0]);
		else //其他错误
			err_sys("unable to lock %s", PATH_PIDFILE);
	}

		/* 4write my PID, leave file open to hold the write lock */
	snprintf(line, sizeof(line), "%ld\n", (long) getpid()); //打印进程pid到line中
	Ftruncate(pidfd, 0); //截断文件长度为0
    Write(pidfd, line, strlen(line)); //把本进程pid写入这个文件中

	/* then do whatever the daemon does ... */

	pause();
	return 0;
}
