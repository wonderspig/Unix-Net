#include	"unpipc.h"

int
main(int argc, char **argv)
{
	mqd_t	mqd;
	void	*ptr;
	size_t	len;
	unsigned int	prio;

	if (argc != 4) //如果命令行参数不等于3,那么报错
		err_quit("usage: mqsend <name> <#bytes> <priority>");
	len = atoi(argv[2]); //消息的长度
	prio = atoi(argv[3]); //消息的优先级

	mqd = Mq_open(argv[1], O_WRONLY); //以写的方式打开消息队列

	ptr = Calloc(len, sizeof(char)); //根据传参的消息长度,动态分配存放消息的缓冲区
	Mq_send(mqd, ptr, len, prio); //向mqd这个消息队列,发送缓冲区ptr中的消息,消息长度为len,消息优先级为prio

	exit(0);
}
