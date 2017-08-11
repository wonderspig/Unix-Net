#include	"unpipc.h"

#define	FIFO1	"/tmp/fifo.1"
#define	FIFO2	"/tmp/fifo.2"

void	client(int, int), server(int, int);

int
main(int argc, char **argv)
{
	int		readfd, writefd;
	pid_t	childpid;

		/* 4create two FIFOs; OK if they already exist */
	if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) //创建第一个FIFO
		err_sys("can't create %s", FIFO1);
	if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) { //创建第二个FIFO
		unlink(FIFO1);
		err_sys("can't create %s", FIFO2);
	}
	//创建一个子进程
	if ( (childpid = Fork()) == 0) {		/* child 在子进程中 */
		readfd = Open(FIFO1, O_RDONLY, 0); //对第一个FIFO读打开
		writefd = Open(FIFO2, O_WRONLY, 0); //对第二个FIFO写打开
		//子进程运行服务器函数:从FIFO1读取文件路径并打开它,读出这个文件的内容并写到FIFO2中
		server(readfd, writefd); 
		exit(0);
	}
		/* 4parent 在父进程中*/
	writefd = Open(FIFO1, O_WRONLY, 0); //对第一个FIFO写打开
	readfd = Open(FIFO2, O_RDONLY, 0); //对第二个FIFO读打开
	//父进程运行客户端函数:把文件名路径写入FIFO1,等待服务器读取文件内容并从FIFO2把文件内容传送过来
	client(readfd, writefd);

	Waitpid(childpid, NULL, 0);		/* wait for child to terminate 回收子进程资源 */

	Close(readfd);
	Close(writefd);
	//删除两个FIFO文件
	Unlink(FIFO1);
	Unlink(FIFO2);
	exit(0);
}
