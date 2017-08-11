#include	"unp.h"

int heartbeat_action(int sock_fd, struct sockaddr *sa, socklen_t salen,
			  u_int value)
{
	struct sctp_paddrparams sp; //套接字选项SCTP_PEER_ADDR_PARAMS需要的结构体
	int siz;

	bzero(&sp,sizeof(sp)); //清空sp结构体
	sp.spp_hbinterval = value; //把调用者希望设置的心搏值填入sp结构体
	memcpy((caddr_t)&sp.spp_address,sa,salen); //把需要设置的对端地址填写入结构体
	Setsockopt(sock_fd,IPPROTO_SCTP,
		   SCTP_PEER_ADDR_PARAMS, &sp, sizeof(sp)); //设置套接字选项,设置心搏
	return(0);
}
