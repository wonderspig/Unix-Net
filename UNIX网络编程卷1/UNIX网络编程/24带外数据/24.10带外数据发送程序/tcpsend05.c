#include	"unp.h"

int
main(int argc, char **argv)
{
	int		sockfd, size;
	char	buff[16384];

	if (argc != 3) //命令行参数不等于2
		err_quit("usage: tcpsend05 <host> <port#>");
	//查找地址,用这个地址的地址族和协议创建套接字,链接这个地址
	sockfd = Tcp_connect(argv[1], argv[2]);

	size = 32768; 
	Setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)); //设置套接字选项SO_SNDBUF(定义发送缓冲区大小)

	Write(sockfd, buff, 16384); //发送16384个字节数据
	printf("wrote 16384 bytes of normal data\n");
	sleep(5);

	Send(sockfd, "a", 1, MSG_OOB); //发送一个字节带外数据
	printf("wrote 1 byte of OOB data\n");

	Write(sockfd, buff, 1024); //发送1024字节数据
	printf("wrote 1024 bytes of normal data\n");

	exit(0);
}
