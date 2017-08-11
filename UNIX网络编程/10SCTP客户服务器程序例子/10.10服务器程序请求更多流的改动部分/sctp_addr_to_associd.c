#include	"unp.h"

//此函数根据套接字sock_fd,和地址sa,返回关联标识
sctp_assoc_t
sctp_address_to_associd(int sock_fd, struct sockaddr *sa, socklen_t salen, sctp_assoc_t assoc_id)
{
	/*
	//SCTP_PEER_ADDR_PARAMS这个套接字选项可以获得关联对端地址的各种参数
	struct sctp_paddrparams sp; //这个SCTP_PEER_ADDR_PARAMS套接字选项的结构体
	int siz;

	siz = sizeof(struct sctp_paddrparams); //计算sp这个结构体的长度
	bzero(&sp,siz); //清空初始化sp结构体
	memcpy(&sp.spp_address,sa,salen); //填入结构体,将会返回关联标识
	//设置SCTP_PEER_ADDR_PARAMS套接字选项,把有地址的sp结构体输入,填写完后会得到关联标识
	sctp_opt_info(sock_fd,0,
		   SCTP_PEER_ADDR_PARAMS, &sp, &siz); 
	*/




	return(assoc_id); //返回关联标识
}
