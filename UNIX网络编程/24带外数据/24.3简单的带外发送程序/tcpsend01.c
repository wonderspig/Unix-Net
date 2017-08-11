#include	"unp.h"

int
main(int argc, char **argv)
{
	int		sockfd;

	if (argc != 3) //如果命令行参数不等于2,则出错退出
		err_quit("usage: tcpsend01 <host> <port#>");
	//根据参数1(主机名),参数2(服务名),查找合适的地址,根据这个地址创建套接字,并连接到这个地址所在的服务器
	sockfd = Tcp_connect(argv[1], argv[2]); //

	Write(sockfd, "123", 3); //发送普通数据
	printf("wrote 3 bytes of normal data\n");
	sleep(1);

	Send(sockfd, "4", 1, MSG_OOB); //发送带外数据
	printf("wrote 1 byte of OOB data\n");
	sleep(1);

	Write(sockfd, "56", 2); //发送普通数据
	printf("wrote 2 bytes of normal data\n");
	sleep(1);

	Send(sockfd, "7", 1, MSG_OOB); //发送带外数据
	printf("wrote 1 byte of OOB data\n");
	sleep(1);

	Write(sockfd, "89", 2); //发送普通数据
	printf("wrote 2 bytes of normal data\n");
	sleep(1);

	exit(0);
}
