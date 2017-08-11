#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	socklen_t		len;
	struct sockaddr	*cliaddr;
	char			buff[MAXLINE];
	time_t			ticks;

	daemon_inetd(argv[0], 0); //初始化为inetd的守护进程

	cliaddr = Malloc(sizeof(struct sockaddr_storage)); //动态分配足够大的空间存放地址结构体
	len = sizeof(struct sockaddr_storage); //计算一下这个地址结构体的大小
	Getpeername(0, cliaddr, &len); //获得对端的协议地址,填写在cliaddr中
	//往日志文件中写入消息,已经连接到哪个地址了
	//此函数把地址结构体中的ip地址和端口翻译成表达式格式
	err_msg("connection from %s", Sock_ntop(cliaddr, len)); 

    ticks = time(NULL); //获得当前日历时间
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); //把日历时间翻译成字符串并打印到buff中
    Write(0, buff, strlen(buff)); //把buff写给客户端

	Close(0);	/* close TCP connection */
	exit(0);
}
