#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2) //如果参数不为1,则出错
		err_quit("usage: udpcli <IPaddress>");

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
	servaddr.sin_family = AF_INET; //ip4地址族
	servaddr.sin_port = htons(SERV_PORT); //绑定端口号
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //把参数中的IP地址翻译成二进制并填写入servaddr.sin_addr

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建ip4,UDP套接字

	dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr)); //执行客户端的处理工作

	exit(0);
}
