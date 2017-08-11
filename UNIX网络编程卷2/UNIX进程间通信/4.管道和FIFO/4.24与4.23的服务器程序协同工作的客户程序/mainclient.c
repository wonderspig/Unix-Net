#include	"fifo.h"

int
main(int argc, char **argv)
{
	int		readfifo, writefifo;
	size_t	len;
	ssize_t	n;
	char	*ptr, fifoname[MAXLINE], buff[MAXLINE];
	pid_t	pid;

		/* 4create FIFO with our PID as part of name */
	pid = getpid(); //获得本进程的PID
	snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid); //用本进程的PID生成一个路径名字符串,存放到fifoname中
	if ((mkfifo(fifoname, FILE_MODE) < 0) && (errno != EEXIST)) //根据生成的路径名创建FIFO
		err_sys("can't create %s", fifoname);

		/* 4start buffer with pid and a blank */
	//buff是发送给服务器的请求,格式是PID+空格+需要打开的文件的路径名
	snprintf(buff, sizeof(buff), "%ld ", (long) pid); //把本进程的PID填写在buff的开头位置
	len = strlen(buff); //计算buff中当前字符串的字符个数(包括末尾的空白字符)
	ptr = buff + len; //ptr指针移动到buff中已存放字符串的"末尾空白+1"位置(此时和前面的PID已经有了一个空格相分割)

		/* 4read pathname */
	Fgets(ptr, MAXLINE - len, stdin); //把标准输入读到的路径名存入指针ptr指向的位置
	len = strlen(buff);		/* fgets() guarantees null byte at end 计算一下当前buff的长度(len包含了末尾的空白字符)*/

		/* 4open FIFO to server and write PID and pathname to FIFO */
	writefifo = Open(SERV_FIFO, O_WRONLY, 0); //打开服务器众所周知的FIFO
	Write(writefifo, buff, len); //把这条格式化完成的请求发送给服务器

		/* 4now open our FIFO; blocks until server opens for writing */
	readfifo = Open(fifoname, O_RDONLY, 0); //以读的方式打开自己创建的FIFO,用来接收服务器返回的消息

		/* 4read from IPC, write to standard output */
	while ( (n = Read(readfifo, buff, MAXLINE)) > 0) //读取服务器发送过来的消息,并复制到标准输出
		Write(STDOUT_FILENO, buff, n);

	Close(readfifo); //关闭我们自己创建的FIFO
	Unlink(fifoname); //删除我们自己创建的FIFO
	exit(0);
}
