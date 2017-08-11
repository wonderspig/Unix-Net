#include	"fifo.h"

void	client(int, int);

int
main(int argc, char **argv)
{
	int		readfd, writefd;

	writefd = Open(FIFO1, O_WRONLY, 0); //打开第一个FIFO
	readfd = Open(FIFO2, O_RDONLY, 0); //打开第二个FIFO
	//客户端函数:把文件名路径写入FIFO1,等待服务器读取文件内容并从FIFO2把文件内容传送过来
	client(readfd, writefd);

	Close(readfd);
	Close(writefd);

	Unlink(FIFO1); //删除FIFO1
	Unlink(FIFO2); //删除FIFO2
	exit(0);
}
