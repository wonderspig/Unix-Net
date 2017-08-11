#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		fd[2], n;
	char	c;
	pid_t	childpid;

	Pipe(fd);		/* assumes a full-duplex pipe (e.g., SVR4) 创建一个管道(全双工)*/
	//创建子进程
	if ( (childpid = Fork()) == 0) {		/* child 在子进程中 */
		sleep(3); //谁3秒来保证父进程先写在读会不会读出自己刚写入的数据
		if ( (n = Read(fd[0], &c, 1)) != 1) //从管道的一端读一个字节
			err_quit("child: read returned %d", n);
		printf("child read %c\n", c); //打印读到的这个字节
		Write(fd[0], "c", 1); //把这个字节写入管道
		exit(0);
	}
		/* 4parent 在父进程中*/
	Write(fd[1], "p", 1); //往管道的一端写入一个字节
	if ( (n = Read(fd[1], &c, 1)) != 1) //从管道的一端读一个字节
		err_quit("parent: read returned %d", n);
	printf("parent read %c\n", c); //把读到的字节打印到标准输出
	exit(0);
}
