#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	socklen_t			len;
	struct sockaddr_un	addr1, addr2;

	if (argc != 2) //如果参数不为1,则出错
		err_quit("usage: unixbind <pathname>");

	sockfd = Socket(AF_LOCAL, SOCK_STREAM, 0); //创建unix域套接字,tcp

	unlink(argv[1]);		/* OK if this fails 解除命令行第一个参数代表的文件链接(先删除再创建) */

	bzero(&addr1, sizeof(addr1)); //初始化清空地址结构体
	addr1.sun_family = AF_LOCAL; //设置为unix域套接字
	strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path)-1); //填写unix域套接字的地址
	Bind(sockfd, (SA *) &addr1, SUN_LEN(&addr1)); //绑定这个填写好的unix域套接字结构

	len = sizeof(addr2); //计算这个unix域套接字结构的大小，用来调用Getsockname
	Getsockname(sockfd, (SA *) &addr2, &len); //从已绑定套接字sockfd中获得地址
	printf("bound name = %s, returned len = %d\n", addr2.sun_path, len); //打印这个地址
	
	exit(0);
}
