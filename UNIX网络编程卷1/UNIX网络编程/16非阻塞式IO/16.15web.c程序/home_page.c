#include	"web.h"

void
home_page(const char *host, const char *fname)
{
	int		fd, n;
	char	line[MAXLINE];

	fd = Tcp_connect(host, SERV);	/* blocking connect() 建立一个与服务器的连接*/

	//把传参进来的fname字符串,按照GET_CMD宏所声明的格式打印成line
	n = snprintf(line, sizeof(line), GET_CMD, fname); 
	Writen(fd, line, n); //把line写入到连接到服务器的套接字(给服务器发出请求)

	for ( ; ; ) {
		//读取服务器的应答,但没有对应答做任何操作,用来检验服务器是否运行正常
		if ( (n = Read(fd, line, MAXLINE)) == 0) //读取服务器的应答,存放到line中
			break;		/* server closed connection 
						   read返回0,服务器已经关闭连接(应答已经读完了) */

		printf("read %d bytes of home page\n", n); //打印读取了多少字节的信息
		/* do whatever with data */
	}
	printf("end-of-file on home page\n");
	Close(fd); //关闭连接到服务器的套接字 
}
