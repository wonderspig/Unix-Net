#include	"unp.h"

#define	SAP_NAME	"sap.mcast.net"	/* default group name and port */
#define	SAP_PORT	"9875"

void	loop(int, socklen_t);

int
main(int argc, char **argv)
{
	int					sockfd;
	const int			on = 1;
	socklen_t			salen;
	struct sockaddr		*sa;

	if (argc == 1) //有一个命令行参数
		//通过众所周知的名字和端口,创建UDP套接字,把找到的地址填写进sa中
		sockfd = Udp_client(SAP_NAME, SAP_PORT, (SA **) &sa, &salen);
	else if (argc == 4) //如果命令行参数有3个,则把第一个参数作为主机名
						//第二个参数作为端口号,创建UDP套接字,把地址填写进sa中
		sockfd = Udp_client(argv[1], argv[2], (SA **) &sa, &salen);
	else
		err_quit("usage: mysdr <mcast-addr> <port#> <interface-name>");
	//套接字选项SO_REUSEADDR,允许在单个主机上运行本程序的多个实例
	Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(sockfd, sa, salen); //捆绑地址sa到套接字上(非必须,但可以过滤掉不需要的UDP数据包)
	//加入多播组
	//sa为需要加入的多播组地址
	//如果参数指定了需要加入多播组的接口名,则把第三个参数传入
	Mcast_join(sockfd, sa, salen, (argc == 4) ? argv[3] : NULL, 0);

	loop(sockfd, salen);	/* receive and print 读取并显示所有声明 */

	exit(0);
}
