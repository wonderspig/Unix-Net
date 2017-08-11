#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建ip4,udp的socket

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family      = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配地址
	servaddr.sin_port        = htons(SERV_PORT); //填写绑定的端口号

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr)); //把填写好的地址绑定到套接字上

	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr)); //在这个函数中，执行服务器的处理工作
}
