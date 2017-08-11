#include	"unpipc.h"

int
main(int argc, char **argv)
{
	if (argc != 2) //如果命令行参数不是一个,则出错
		err_quit("usage: pipeconf <pathname>");

	printf("PIPE_BUF = %ld, OPEN_MAX = %ld\n",
		   Pathconf(argv[1], _PC_PIPE_BUF), Sysconf(_SC_OPEN_MAX)); //打印传参路径的两个限制值
	exit(0);
}
