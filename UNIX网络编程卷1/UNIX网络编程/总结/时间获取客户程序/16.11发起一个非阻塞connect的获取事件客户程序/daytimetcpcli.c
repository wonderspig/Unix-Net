#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	if (argc != 2) //如果没有一个参数,则出错退出
		err_quit("usage: a.out <IPaddress>");

	//创建套接字
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr)); //清空servaddr的内存
	servaddr.sin_family = AF_INET; //地址族设置为IPv4
	servaddr.sin_port   = htons(13);	/* daytime server 把端口号转换成网络字节续并填入servaddr */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) //把字符串格式的地址转换后填入servaddr
		err_quit("inet_pton error for %s", argv[1]);

	/*
	//地址填写完毕，链接这个地址
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) //把地址转换成通用套接字格式后,进行连接
		err_sys("connect error");
	*/

	//前三个参数和connect一样，第四个参数为0表示不给select设置超时秒数
	if (connect_nonb(sockfd, (SA*) &servaddr, sizeof(servaddr), 0) < 0)
		err_sys("connect error");

	//循环从套接字中读数据,直到返回0或者小于0的数
	//返回0说明读完了,返回小于0的数说明出错
	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) { 
		recvline[n] = 0;	/* null terminate 加上末尾的空字符串 */
		if (fputs(recvline, stdout) == EOF) //把从套接字中读道德数据,压入标准输出
			err_sys("fputs error");
	}
	if (n < 0) //处理read出错的情况
		err_sys("read error");

	exit(0);
}
