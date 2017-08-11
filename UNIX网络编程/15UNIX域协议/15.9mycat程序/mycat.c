#include	"unp.h"

int		my_open(const char *, int);

int
main(int argc, char **argv)
{
	int		fd, n;
	char	buff[BUFFSIZE];

	if (argc != 2) //如果命令行参数不为1,则出错退出
		err_quit("usage: mycat <pathname>");

	//my_open函数fork exec一个新的程序,由这个程序打开命令行指定的描述符,再把这个描述符传递给父进程
	//通过my_open函数返回
	if ( (fd = my_open(argv[1], O_RDONLY)) < 0) //在my_open中打开描述符
		err_sys("cannot open %s", argv[1]);

	while ( (n = Read(fd, buff, BUFFSIZE)) > 0) //从已经打开的文件描述符中读
		Write(STDOUT_FILENO, buff, n); //写入标准输出

	exit(0);
}
