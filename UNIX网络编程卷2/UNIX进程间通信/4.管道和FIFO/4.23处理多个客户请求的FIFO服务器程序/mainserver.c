#include	"fifo.h"

void	server(int, int);

int
main(int argc, char **argv)
{
	int		readfifo, writefifo, dummyfd, fd;
	char	*ptr, buff[MAXLINE], fifoname[MAXLINE];
	pid_t	pid;
	ssize_t	n;

		/* 4create server's well-known FIFO; OK if already exists */
	if ((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST)) //服务器创建一个众所周知的FIFO
		err_sys("can't create %s", SERV_FIFO);

		/* 4open server's well-known FIFO for reading and writing */
	readfifo = Open(SERV_FIFO, O_RDONLY, 0); //以只读的方式打开该FIFO
	dummyfd = Open(SERV_FIFO, O_WRONLY, 0);		/* never used 以只写的方式打开该FIFO(只写的FIFO描述符从不使用) */

	while ( (n = Readline(readfifo, buff, MAXLINE)) > 0) { //从FIFO中读一行
		if (buff[n-1] == '\n') //如果缓冲区最后一位是'\n',那么把'\n'替换成'\0'
			n--;			/* delete newline from readline() 删除'\n' */
		buff[n] = '\0';		/* null terminate pathname 在缓冲区的最后一位填写为'\0' */

		if ( (ptr = strchr(buff, ' ')) == NULL) { //查找字符串buff中首次出现空格的位置
			err_msg("bogus request: %s", buff); //字符串中没有找到空格,忽略这个请求,继续循环从FIFO中读
			continue;
		}

		*ptr++ = 0;			/* null terminate PID, ptr = pathname 让指针ptr指向路径名的开头 */
		pid = atol(buff); //buff数组仍然指向数组开始处的地址,此处到底一个空格为止,是客户进程的PID,把这个PID从表达式格式转换成整数形态
		snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid); //根据这个PID格式化一个路径字符串
		if ( (writefifo = open(fifoname, O_WRONLY, 0)) < 0) { //打开这个路径所代表的FIFO
			err_msg("cannot open: %s", fifoname); //打不开就说明客户进程没有创建相应的FIFO
			continue;
		}

		if ( (fd = open(ptr, O_RDONLY)) < 0) { //ptr已经指向了请求字符串中的路径名的开始处位置,这个路径名就是客户要求打开的文件
				/* 4error: must tell client */
			snprintf(buff + n, sizeof(buff) - n, ": can't open, %s\n",
					 strerror(errno)); //打开这个文件失败就传送客户进程这条出错消息
			n = strlen(ptr);
			Write(writefifo, ptr, n);
			Close(writefifo);
	
		} else { //客户所要求的文件打开成功
				/* 4open succeeded: copy file to FIFO */
			while ( (n = Read(fd, buff, MAXLINE)) > 0) //循环读取该文件的内容,并复制到FIFO中
				Write(writefifo, buff, n);
			Close(fd);
			Close(writefifo);
		}
	}
	exit(0);
}
