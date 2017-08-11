#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd,fd;
	struct sockaddr_un	cliaddr, servaddr;
	char	template[] = "/tmp/dirXXXXXX";

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0); //创建套接字,unix域,udp

	fd = mkstemp(template);
	close(fd);
	unlink(template);

	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us 清空客户端地址结构体*/
	cliaddr.sun_family = AF_LOCAL; //地址族为unix域
	strcpy(cliaddr.sun_path, template); //填写临时路径到地址结构体中

	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr)); //绑定填写好的地址结构体到套接字

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address 清空服务器地址结构体*/
	servaddr.sun_family = AF_LOCAL; //地址族为unix域
	strcpy(servaddr.sun_path, UNIXDG_PATH); //填写服务器众所周知的路径到地址结构体

	dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr)); //此函数中完成服务器历程

	exit(0);
}
