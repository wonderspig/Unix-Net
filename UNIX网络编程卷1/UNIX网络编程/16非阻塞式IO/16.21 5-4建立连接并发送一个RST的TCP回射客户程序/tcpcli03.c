#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct linger		ling;
	struct sockaddr_in	servaddr;

	if (argc != 2) //如果参数不为1,则出错
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0); //创建套接字,ip4,tcp

	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family = AF_INET; //ip4地址族
	servaddr.sin_port = htons(SERV_PORT); //填入端口号
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //翻译二进制地址为表达式格式,填入servaddr.sin_addr

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr)); //连接服务器

	ling.l_onoff = 1;		/* cause RST to be sent on close() */
	ling.l_linger = 0;
	//用套接字选项向服务器发送RST,避免time_wait状态
	Setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)); 
	Close(sockfd);

	exit(0);
}
