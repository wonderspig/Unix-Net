#include	"unp.h"

int
sctp_bind_arg_list(int sock_fd, char **argv, int argc)
{
	struct addrinfo *addr;
	char *bindbuf, *p, portbuf[10];
	int addrcnt=0;
	int i;
	//argc代表有多少个地址需要绑定到套接字,一个套接字用一个sockaddr_storage结构体的大小存放
	//所以一共需要argc*sizeof(struct sockaddr_storage)大小的空间
	bindbuf = (char *)Calloc(argc, sizeof(struct sockaddr_storage));
	p = bindbuf; //把指向存放地址列表缓冲区的指针浅复制一份,
	sprintf(portbuf, "%d", SERV_PORT); //把端口号打印成表达式格式,存放在portbuf中(为了调用getaddrinfo函数的外包函数)
	for( i=0; i<argc; i++ ) { //有几个参数(几个地址)就循环几次,每次把一个地址放入缓冲区中
		//Host_serv函数会根据,主机名(地址名),端口号(服务名),地址族AF_UNSPEC,协议SCTP,来获得addrinfo结构体
		addr = Host_serv(argv[i], portbuf, AF_UNSPEC, SOCK_SEQPACKET);
		memcpy(p, addr->ai_addr, addr->ai_addrlen); //把addrinfo中的地址复制到缓冲区中
		freeaddrinfo(addr); //释放Host_serv中分配的空间
		addrcnt++; //地址计数+1
		p += addr->ai_addrlen; //指针指向地址末尾的空白处,以便下一个地址从这个地方开始填写
	}
	//绑定填写好的缓冲区中的地址,标志SCTP_BINDX_ADD_ADDR为添加地址
	Sctp_bindx(sock_fd,(SA *)bindbuf,addrcnt,SCTP_BINDX_ADD_ADDR);
	free(bindbuf); //清理缓冲区
	return(0);
}
