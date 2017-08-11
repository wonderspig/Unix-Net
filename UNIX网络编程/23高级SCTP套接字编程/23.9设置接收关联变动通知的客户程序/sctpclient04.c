#include	"unp.h"

int
main(int argc, char **argv)
{
	int sock_fd;
	struct sockaddr_in servaddr;
	struct sctp_event_subscribe evnts;

	if(argc != 2) //命令行参数不为1,则出错退出
		err_quit("Missing host argument - use '%s host'\n",
		       argv[0]);
	//创建IP4套接字,SCTP协议
    sock_fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
/* include mod_client04 */
	bzero(&evnts, sizeof(evnts)); //清空事件结构体
	evnts.sctp_data_io_event = 1; //收取sctp_sndrcvinfo结构体
	evnts.sctp_association_event = 1; //关心关联变动事件
	Setsockopt(sock_fd,IPPROTO_SCTP, SCTP_EVENTS,
		   &evnts, sizeof(evnts)); //把关心的事件设置进套接字

	sctpstr_cli(stdin,sock_fd,(SA *)&servaddr,sizeof(servaddr)); 
/* end mod_client04 */
	close(sock_fd);
	return(0);
}
