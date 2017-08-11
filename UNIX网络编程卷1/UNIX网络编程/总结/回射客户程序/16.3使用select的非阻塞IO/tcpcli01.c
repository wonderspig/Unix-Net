#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2) //如果没有一个参数,返回出错
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0); //创建ip4,TCP套接字

	bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
	servaddr.sin_family = AF_INET; //地址族为ip4
	servaddr.sin_port = htons(SERV_PORT); //填写端口号
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //把命令行参数翻译成二进制地址

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr)); //连接这个填写完成的地址

	str_cli(stdin, sockfd);		/* do it all 在这个函数中做一切客户端的事情*/

	exit(0);
}
