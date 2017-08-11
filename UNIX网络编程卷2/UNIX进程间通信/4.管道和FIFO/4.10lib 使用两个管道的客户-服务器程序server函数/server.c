#include	"unpipc.h"

void
server(int readfd, int writefd)
{
	int		fd;
	ssize_t	n;
	char	buff[MAXLINE+1];

		/* 4read pathname from IPC channel */
	if ( (n = Read(readfd, buff, MAXLINE)) == 0) //从连接到客户端的管道中读取数据(父进程)
		err_quit("end-of-file while reading pathname");
	buff[n] = '\0';		/* null terminate pathname 缓冲区以空字符结尾*/

	if ( (fd = open(buff, O_RDONLY)) < 0) { //通过客户端(父进程传送过来的文件路径以读的方式打开文件)
			/* 4error: must tell client 进入此分支代表打开失败 */
		snprintf(buff + n, sizeof(buff) - n, ": can't open, %s\n",
				 strerror(errno)); //格式化失败信息的字符串
		n = strlen(buff);
		Write(writefd, buff, n); //把读取文件失败的信息传送给客户端

	} else {
			/* 4open succeeded: copy file to IPC channel */
		while ( (n = Read(fd, buff, MAXLINE)) > 0) //循环从已经打开的文件中读取消息,当结果小于等于0(意味着文件读取结束)
			Write(writefd, buff, n); //把读到的消息都通过管道发送给客户端(父进程)
		Close(fd);
	}
}
