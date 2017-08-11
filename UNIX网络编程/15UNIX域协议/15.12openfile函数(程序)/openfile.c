#include	"unp.h"

int
main(int argc, char **argv)
{
	int		fd;

	if (argc != 4) //如果参数不为3个,则出错
		err_quit("openfile <sockfd#> <filename> <mode>");

	//atoi把字符串转换成int类型的数字
	//第二个参数为需要打开文件的路径,第三个参数为需要打开文件的权限
	if ( (fd = open(argv[2], atoi(argv[3]))) < 0) 
		exit( (errno > 0) ? errno : 255 );

	if (write_fd(atoi(argv[1]), "", 1, fd) < 0) //把已打开的描述符发送给父进程
		exit( (errno > 0) ? errno : 255 );

	exit(0);
}
