#include	"unp.h"

//此函数根据套接字sock_fd和地址to获得关联标识
//再通过关联标识获得本端用于向对端发送的最大流的数目
int 
sctp_get_no_strms(int sock_fd,struct sockaddr *to, socklen_t tolen, sctp_assoc_t assoc_id)
{
	int retsz;
	struct sctp_status status; //此结构体为SCTP_STATUS这个套接字选项的结构体
	retsz = sizeof(status);	//计算一下status这个结构体的长度
	bzero(&status,sizeof(status)); //初始化清空这个结构体

	//sctp_address_to_associd这个函数可以取得sock_fd这个套接字上的关联标识
	status.sstat_assoc_id = sctp_address_to_associd(sock_fd,to,tolen,assoc_id);
	//设置SCTP_STATUS套接字选项,并填写status这个结构,通过填写进去的关联标识,获得本端用于向对端发送的最大流的数目
	/*
	Getsockopt(sock_fd,IPPROTO_SCTP, SCTP_STATUS,
		   &status, &retsz);
	*/
	//在linux中证明,为了增加移植性,应当使用sctp_opt_info函数

	sctp_opt_info(sock_fd,assoc_id,SCTP_STATUS,&status,&retsz);
	
	return(status.sstat_outstrms); //返回本端用于向对端发送的流的数目
}
