#include	"unp.h"

void	recv_all(int, socklen_t);
void	send_all(int, SA *, socklen_t);

int
main(int argc, char **argv)
{
	int					sendfd, recvfd;
	const int			on = 1;
	socklen_t			salen;
	struct sockaddr		*sasend, *sarecv;

	if (argc != 3) //命令行参数不是2个
		err_quit("usage: sendrecv <IP-multicast-address> <port#>");
	//创建发送套接字
	//命令行参数1为主机名,命令行参数2为端口号,创建套接字,地址填写在sasend中
	sendfd = Udp_client(argv[1], argv[2], (SA **) &sasend, &salen);
	//创建接收套接字
	//用创建发送套接字的地址的地址族,UDP协议,创建接收套接字
	recvfd = Socket(sasend->sa_family, SOCK_DGRAM, 0);
	//设置套接字选项,SO_REUSEADDR允许主机运行本程序的多个实例
	Setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	//动态分配存放接受套接字的地址结构体空间
	sarecv = Malloc(salen);
	memcpy(sarecv, sasend, salen); //把发送套接字的地址结构复制到接收套接字的地址结构体中
	Bind(recvfd, sarecv, salen); //绑定这个地址到接收套接字上
	//在接收套接字recvfd上,加入多播组地址sasend,任意接口名字和索引号
	Mcast_join(recvfd, sasend, salen, NULL, 0);
	Mcast_set_loop(sendfd, 0); //回馈设置为0

	if (Fork() == 0) //创建子进程,在子进程中
		recv_all(recvfd, salen);		/* child -> receives 接收套接字中的UDP数据包*/
	//传入多播目的地址和端口号的结构体
	send_all(sendfd, sasend, salen);	/* parent -> sends 往发送套接字中,发送数据*/
}
