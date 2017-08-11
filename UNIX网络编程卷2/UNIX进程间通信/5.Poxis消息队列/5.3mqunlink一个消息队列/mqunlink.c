#include	"unpipc.h"

int
main(int argc, char **argv)
{
	if (argc != 2) //如果命令行参数不为1个,则出错
		err_quit("usage: mqunlink <name>");

	Mq_unlink(argv[1]); //删除命令行参数所指定的那个消息队列

	exit(0);
}
