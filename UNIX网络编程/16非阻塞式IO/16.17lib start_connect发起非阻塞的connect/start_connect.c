#include	"web.h"

void
start_connect(struct file *fptr)
{
	int				fd, flags, n;
	struct addrinfo	*ai;

	//此函数通过主机名,端口号,地址族,和协议来获取地址结构体
	ai = Host_serv(fptr->f_host, SERV, 0, SOCK_STREAM); 
	//通过Host_serv所返回的文件所在的服务器的地址信息,来建立连接到服务器的套接字(这个套接字用来下载这个文件)
	fd = Socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	fptr->f_fd = fd; //把将要连接到这个文件的套接字存入文件的结构体中
	printf("start_connect for %s, fd %d\n", fptr->f_name, fd); //打印这个文件的文件名,和所对应的fd

		/* 4Set socket nonblocking */
	flags = Fcntl(fd, F_GETFL, 0);
	Fcntl(fd, F_SETFL, flags | O_NONBLOCK); //设置连接到这个文件的套接字为非阻塞

		/* 4Initiate nonblocking connect to the server. 对这个套接字进行连接 */
	if ( (n = connect(fd, ai->ai_addr, ai->ai_addrlen)) < 0) { 
		if (errno != EINPROGRESS) //这个错误代表三路握手正在进行
			err_sys("nonblocking connect error"); //别的错误就出错退出
		fptr->f_flags = F_CONNECTING; //文件的标志设置为正在连接
		//在读描述符集和写描述符集中打开这个文件的描述符位
		FD_SET(fd, &rset);			/* select for reading and writing */
		FD_SET(fd, &wset);
		if (fd > maxfd) //更新select参数中的最大文件描述符参数
			maxfd = fd;

	} else if (n >= 0)				/* connect is already done 连接已经成功 */
		write_get_cmd(fptr);	/* write() the GET command 向服务器发送GET命令*/
}
