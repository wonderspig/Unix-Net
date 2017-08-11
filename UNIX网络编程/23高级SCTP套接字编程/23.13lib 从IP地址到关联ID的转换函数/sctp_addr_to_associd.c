#include	"unp.h"

sctp_assoc_t
sctp_address_to_associd(int sock_fd, struct sockaddr *sa, socklen_t salen)
{
	struct sctp_paddrparams sp; //套接字SCTP_PEER_ADDR_PARAMS所需要的结构体
	int siz;

	siz = sizeof(struct sctp_paddrparams); //计算sctp_paddrparams结构体的大小
	bzero(&sp,siz); //清空sctp_paddrparams结构体
	memcpy(&sp.spp_address,sa,salen); //把给定的对端地址填入sctp_paddrparams结构体
	sctp_opt_info(sock_fd,0,
		   SCTP_PEER_ADDR_PARAMS, &sp, &siz); //通过套接字选项获得该对端地址的详细信息
	return(sp.spp_assoc_id); //把关联id返回
}
