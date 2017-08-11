#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	servaddr, cliaddr;

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0); //创建套接字

	unlink(UNIXDG_PATH); //先删除这个socket文件(如果存在)
	bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
	servaddr.sun_family = AF_LOCAL; //地址族为unix域
	strcpy(servaddr.sun_path, UNIXDG_PATH); //填写服务器众所周知的地址进地址结构体

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr)); //绑定套接字,产生socket文件

	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr)); //在此函数中完成服务器历程
}
