#include	"unp.h"

int
main(int argc, char **argv)
{
	int		sockfd;

	if (argc != 3) //命令行参数不等于2
		err_quit("usage: tcpsend06 <host> <port#>");
	//查找地址,用这个地址的地址族和协议创建套接字,链接这个地址
	sockfd = Tcp_connect(argv[1], argv[2]);

	Write(sockfd, "123", 3);
	printf("wrote 3 bytes of normal data\n");

	Send(sockfd, "4", 1, MSG_OOB); //发送第一个带外数据
	printf("wrote 1 byte of OOB data\n");

	Write(sockfd, "5", 1);
	printf("wrote 1 byte of normal data\n");

	Send(sockfd, "6", 1, MSG_OOB); //发送第二个带外数据
	printf("wrote 1 byte of OOB data\n");

	Write(sockfd, "7", 1);
	printf("wrote 1 byte of normal data\n");

	exit(0);
}
