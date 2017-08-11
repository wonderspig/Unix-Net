#include	"unp.h"

int
main(int argc, char **argv)
{
	int					i, sockfd[5];
	struct sockaddr_in	servaddr;

	if (argc != 2) //如果参数不是1个,则出错退出
		err_quit("usage: tcpcli <IPaddress>");

	for (i = 0; i < 5; i++) { //循环5次
		sockfd[i] = Socket(AF_INET, SOCK_STREAM, 0); //创建5个监听套接字

		bzero(&servaddr, sizeof(servaddr)); //清空套接字地址结构体
		servaddr.sin_family = AF_INET; //ip4地址族
		servaddr.sin_port = htons(SERV_PORT); //填入端口号
		Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //把命令行参数转换成二进制地址填入结构体

		Connect(sockfd[i], (SA *) &servaddr, sizeof(servaddr)); //连接已经填写好的结构体地址
	}

	str_cli(stdin, sockfd[0]);		/* do it all 这个函数中做客户端所做的所有事情*/

	exit(0);
}
