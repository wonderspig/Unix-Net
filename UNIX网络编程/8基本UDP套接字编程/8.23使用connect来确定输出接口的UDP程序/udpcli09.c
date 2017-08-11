#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	socklen_t			len;
	struct sockaddr_in	cliaddr, servaddr;

	if (argc != 2) //如果命令行参数不为1,则出错返回
		err_quit("usage: udpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建IP4,UDP套接字

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体(对端)
	servaddr.sin_family = AF_INET; //IP4地址族
	servaddr.sin_port = htons(SERV_PORT); //绑定端口号
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //转换命令参数的地址为二进制地址,并填入地址结构体

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr)); //connect对端的地址(同时内核自动指派本地ip和端口)

	len = sizeof(cliaddr); //计算本地端的地址结构体长度
	Getsockname(sockfd, (SA *) &cliaddr, &len); //获得套接字sockfd上本地端的IP地址和端口
	//打印这个IP地址和端口号(二进制转换成字符串)
	printf("local address %s\n", Sock_ntop((SA *) &cliaddr, len)); 

	exit(0);
}
