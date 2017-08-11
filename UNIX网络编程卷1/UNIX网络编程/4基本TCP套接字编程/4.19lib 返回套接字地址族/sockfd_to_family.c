/* include sockfd_to_family */
#include	"unp.h"

int
sockfd_to_family(int sockfd)
{
	struct sockaddr_storage ss; //新的通用套接字地址结构
	socklen_t	len; //地址结构的长度

	len = sizeof(ss);
	//获得在sockfd上面的本地端的地址端口等信息
	if (getsockname(sockfd, (SA *) &ss, &len) < 0) 
		return(-1);
	return(ss.ss_family); //返回地址结构体中的地址族信息
}
/* end sockfd_to_family */

int
Sockfd_to_family(int sockfd)
{
	int		rc;

	if ( (rc = sockfd_to_family(sockfd)) < 0)
		err_sys("sockfd_to_family error");

	return(rc);
}
