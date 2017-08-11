/* include recvfrom_flags1 */
#include	"unp.h"
#include	<sys/param.h>		/* ALIGN macro for CMSG_NXTHDR() macro */

ssize_t
recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp,
			   SA *sa, socklen_t *salenptr, struct unp_in_pktinfo *pktp)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	ssize_t			n;

#ifdef	HAVE_MSGHDR_MSG_CONTROL //定义了HAVE_MSGHDR_MSG_CONTROL宏,支持msg_control成员
	struct cmsghdr	*cmptr;
	union {
	  struct cmsghdr	cm;
	  //辅助数据的存放空间整个的大小(这个辅助数据会存放一个in_addr结构+unp_in_pktinfo架构+头部控制数据+前后填充数据)
	  char				control[CMSG_SPACE(sizeof(struct in_addr)) +
								CMSG_SPACE(sizeof(struct unp_in_pktinfo))];
	} control_un;

	msg.msg_control = control_un.control; //给辅助数据头附值
	msg.msg_controllen = sizeof(control_un.control); //辅助数据报头的大小
	msg.msg_flags = 0; //标志位清零
#else //没有定义宏HAVE_MSGHDR_MSG_CONTROL,不支持msg_control成员
	  //清零msg结构体
	bzero(&msg, sizeof(msg));	/* make certain msg_accrightslen = 0 */
#endif

	msg.msg_name = sa; //把目标端的地址填入
	msg.msg_namelen = *salenptr; //目标端地址的长度
	iov[0].iov_base = ptr; //接收缓冲区
	iov[0].iov_len = nbytes; //接收缓冲区长度
	msg.msg_iov = iov; //把接收缓冲区指针数组填入msg
	msg.msg_iovlen = 1; //数组个数为1
	//fd上已经绑定饿地址
	if ( (n = recvmsg(fd, &msg, *flagsp)) < 0) //从fd中接收消息,填写msg
		return(n);

	*salenptr = msg.msg_namelen;	/* pass back results 返回结果 */
	if (pktp) //如果调用者提供了unp_in_pktinfo这个结构的指针
		bzero(pktp, sizeof(struct unp_in_pktinfo));	/* 0.0.0.0, i/f = 0 把它清零:0.0.0.0 */
/* end recvfrom_flags1 */

/* include recvfrom_flags2 */
#ifndef	HAVE_MSGHDR_MSG_CONTROL //如果不支持msg_control成员
	*flagsp = 0;					/* pass back results 返回0给调用者 */
	return(n); //返回recvmsg读到的字节数
#else

	*flagsp = msg.msg_flags;		/* pass back results 把msg结构体中的标志返回给调用者 */
	//msg.msg_controllen < sizeof(struct cmsghdr)表示没有控制信息
	//MSG_CTRUNC表示控制信息被截断
	//调用者没有提供unp_in_pktinfo结构体指针
	//满足任何一个条件,函数就返回
	if (msg.msg_controllen < sizeof(struct cmsghdr) ||
		(msg.msg_flags & MSG_CTRUNC) || pktp == NULL)
		return(n);
	//取得msg结构体msg_control成员所关联的第一个辅助数据并依次遍历进行循环
	for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL;
		 cmptr = CMSG_NXTHDR(&msg, cmptr)) {
//每循环到一个辅助数据
#ifdef	IP_RECVDSTADDR //IP_RECVDSTADDR宏代表取目的IP地址
		if (cmptr->cmsg_level == IPPROTO_IP &&
			cmptr->cmsg_type == IP_RECVDSTADDR) { 

			memcpy(&pktp->ipi_addr, CMSG_DATA(cmptr),
				   sizeof(struct in_addr)); //取出辅助数据中的ip地址
			continue;
		}
#endif

#ifdef	IP_RECVIF //IP_RECVIF宏代表返回接口索引
		if (cmptr->cmsg_level == IPPROTO_IP &&
			cmptr->cmsg_type == IP_RECVIF) {
			struct sockaddr_dl	*sdl;

			sdl = (struct sockaddr_dl *) CMSG_DATA(cmptr); //取出辅助数据中的链路层地址
			pktp->ipi_ifindex = sdl->sdl_index; //把接口索引号填入调用者提供的结构体中
			continue;
		}
#endif
		err_quit("unknown ancillary data, len = %d, level = %d, type = %d",
				 cmptr->cmsg_len, cmptr->cmsg_level, cmptr->cmsg_type);
	}
	return(n);
#endif	/* HAVE_MSGHDR_MSG_CONTROL */
}
/* end recvfrom_flags2 */

ssize_t
Recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp,
			   SA *sa, socklen_t *salenptr, struct unp_in_pktinfo *pktp)
{
	ssize_t		n;

	n = recvfrom_flags(fd, ptr, nbytes, flagsp, sa, salenptr, pktp);
	if (n < 0)
		err_quit("recvfrom_flags error");

	return(n);
}
