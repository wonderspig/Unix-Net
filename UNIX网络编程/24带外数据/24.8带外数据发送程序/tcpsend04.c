#include	"unp.h"

int
main(int argc, char **argv)
{
	int		sockfd;

	if (argc != 3)
		err_quit("usage: tcpsend04 <host> <port#>");
	//通过命令行参数1(主机名),参数2(端口号)查询IP地址,通过这个地址族和协议创建套接字,并连接到这个地址
	sockfd = Tcp_connect(argv[1], argv[2]);

	Write(sockfd, "123", 3); //发送3个字节数据
	printf("wrote 3 bytes of normal data\n");

	Send(sockfd, "4", 1, MSG_OOB); //发送1个字节带外数据
	printf("wrote 1 byte of OOB data\n");

	Write(sockfd, "5", 1); //发送5个字节数据
	printf("wrote 1 byte of normal data\n");

	exit(0);
}
