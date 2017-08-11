#include	"unp.h"

int
main(int argc, char **argv)
{
	int sock_fd;
	struct sockaddr_in servaddr;
	struct sctp_event_subscribe evnts;
	int echo_to_all=0;

	if(argc < 2) //如果命令行参数小于1个,则出错退出
		err_quit("Missing host argument - use '%s host [echo]'\n",
		       argv[0]);
	if(argc > 2) { //如果命令行参数大于1个,打印提示消息,重置标志量
		printf("Echoing messages to all streams\n"); //回射到全部
		echo_to_all = 1;
	}
    sock_fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP); //创建套接字,IP4,sctp
	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配ip地址
	servaddr.sin_port = htons(SERV_PORT); //填写端口号
	//把命令行参数传递进来的标志量转换成二进制地址填写入地址结构体
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); 

	bzero(&evnts, sizeof(evnts)); //初始化清空事件结构体
	evnts.sctp_data_io_event = 1; //关心填写sockaddr_in这个结构的事件
	Setsockopt(sock_fd,IPPROTO_SCTP, SCTP_EVENTS,
		   &evnts, sizeof(evnts)); //设置套接字选项,关心已经设置的事件
	if(echo_to_all == 0) //说明参数为1
		sctpstr_cli(stdin,sock_fd,(SA *)&servaddr,sizeof(servaddr));
	else //参数大于1
		sctpstr_cli_echoall(stdin,sock_fd,(SA *)&servaddr,sizeof(servaddr));
	Close(sock_fd);
	return(0);
}
