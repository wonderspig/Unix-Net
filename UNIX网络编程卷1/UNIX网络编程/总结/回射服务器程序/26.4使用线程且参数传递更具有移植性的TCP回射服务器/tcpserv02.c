#include	"unpthread.h"

static void	*doit(void *);		/* each thread executes this function */

int
main(int argc, char **argv)
{
	int				listenfd, *iptr;
	pthread_t		tid;
	socklen_t		addrlen, len;
	struct sockaddr	*cliaddr;

	if (argc == 2) //命令行参数等于1
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(NULL, argv[1], &addrlen); 
	else if (argc == 3) //命令行参数等于2
		//根据主机名和端口号,查找合适的IP地址创建套接字,并bind找到的地址在套接字上面,并今听着歌套接字
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		listenfd = 0;
		addrlen = 0;
		err_quit("usage: tcpserv01 [ <host> ] <service or port>");

	cliaddr = Malloc(addrlen); //为存放客户端地址的结构体分配空间

	for ( ; ; ) {
		len = addrlen; //客户端地址结构体的大小
		iptr = Malloc(sizeof(int)); //分配一个int大小的空间(存放文件描述符),每次循环分配一个新的空间
		*iptr = Accept(listenfd, cliaddr, &len); //把连接到客户的文件描述符放入动态生成的内存空间
		Pthread_create(&tid, NULL, &doit, iptr); //把新分配的内存空间的指针传给线程
	}
}

static void *
doit(void *arg)
{
	int		connfd;

	connfd = *((int *) arg); //强转成int类型的指针,在取出指针里的数据(描述符)
	free(arg); //数据已经取出,释放空间

	Pthread_detach(pthread_self()); //本线程成为分离线程
	str_echo(connfd);		/* same function as before 在这个函数中为客户提供服务 */
	Close(connfd);			/* done with connected socket 关闭描述符(线程结束不会自动关闭描述符)*/
	return(NULL);
}
