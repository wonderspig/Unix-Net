#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	struct sockaddr_in	servaddr;
	char				buff[MAXLINE];
	time_t				ticks;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建一个套接字,用来监听的套接字

	bzero(&servaddr, sizeof(servaddr)); //清空这块servaddr内存
	servaddr.sin_family      = AF_INET; //IPv4地址族
	//INADDR_ANY一般定义为0值,表示任意地址
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //把任意地址转换成网络字节序
	servaddr.sin_port        = htons(13);	/* daytime server 把端口转换成网络字节序 */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //把地址强转成通用网络地址并绑定

	Listen(listenfd, LISTENQ); //监听这个已经绑定地址的套接字,LISTENQ为客户的最多排队个数

	for ( ; ; ) {
		connfd = Accept(listenfd, (SA *) NULL, NULL); //连接响应客户端的请求,并获得连接客户端的套接字

        ticks = time(NULL); //获得当前日历时间
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); //把获取的日历时间转换成字符串
        Write(connfd, buff, strlen(buff)); //把字符串写入连接客户端的套接字

		Close(connfd); //关闭连接客户端的套接字
	}
}
