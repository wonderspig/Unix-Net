#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd, n;
	struct sockaddr_in6	servaddr;
	char				recvline[MAXLINE + 1];

	if (argc != 2) //如果参数不等于1,则出错
		err_quit("usage: a.out <IPaddress>");

	if ( (sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) //创建IPv6的，有链接的套接字
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr)); //清空servaddr这块内存
	servaddr.sin6_family = AF_INET6; //地址族定义为IPv6
	servaddr.sin6_port   = htons(13);	/* daytime server 端口号转换成网络字节续后填入servaddr结构 */
	//把参数的字符串形式地址转换成二进制格式并填入servaddr
	if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0) 
		err_quit("inet_pton error for %s", argv[1]);
	//用connect链接填写好的servaddr地址
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) { //用循环从socket中读,如果小于等于0,就跳出循环
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF) //把读到的字符串打印到标准输出
			err_sys("fputs error");
	}
	if (n < 0) //小于0代表read出错
		err_sys("read error");

	exit(0);
}
