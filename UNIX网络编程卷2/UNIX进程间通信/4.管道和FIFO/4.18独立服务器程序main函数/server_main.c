#include	"fifo.h"

void	server(int, int);

int
main(int argc, char **argv)
{
	int		readfd, writefd;

		/* 4create two FIFOs; OK if they already exist */
	if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) //创建第一个FIFO
		err_sys("can't create %s", FIFO1);
	if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) { //创建第二个FIFO
		unlink(FIFO1);
		err_sys("can't create %s", FIFO2);
	}

	readfd = Open(FIFO1, O_RDONLY, 0); //以读的方式打开第一个FIFO
	writefd = Open(FIFO2, O_WRONLY, 0); //以写的方式打开第二个FIFO
	//服务器函数:从FIFO1读取文件路径并打开它,读出这个文件的内容并写到FIFO2中
	server(readfd, writefd);
	exit(0);
}
