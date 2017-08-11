#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	servaddr;

	sockfd = Socket(AF_LOCAL, SOCK_STREAM, 0); //创建套接字,unix域,tcp

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sun_family = AF_LOCAL; //设置为unix域套接字
	strcpy(servaddr.sun_path, UNIXSTR_PATH); //填写众所周知的服务器路径名到地址结构体

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr)); //用填写好的地址链接服务器

	str_cli(stdin, sockfd);		/* do it all 客户端历程在这个函数中完成*/

	exit(0);
}
