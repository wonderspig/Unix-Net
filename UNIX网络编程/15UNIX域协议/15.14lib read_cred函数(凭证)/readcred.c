#include	"unp.h"

#define	CONTROL_LEN	(sizeof(struct cmsghdr) + sizeof(struct cmsgcred))

ssize_t
read_cred(int fd, void *ptr, size_t nbytes, struct cmsgcred *cmsgcredptr)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	char			control[CONTROL_LEN];
	int				n;

	msg.msg_name = NULL; //父子进程字节流链接，不需要关系地址
	msg.msg_namelen = 0; //同理
	iov[0].iov_base = ptr; //接受缓冲区放入iov结构体数组
	iov[0].iov_len = nbytes; //这个缓冲区的大小
	msg.msg_iov = iov; //把填写好的iov缓冲区放入msg结构体
	msg.msg_iovlen = 1; //iov结构体数组的成员个数为1
	msg.msg_control = control; //指向辅助数据的指针
	msg.msg_controllen = sizeof(control); //辅助数据的大小
	msg.msg_flags = 0; //标志设置为0

	if ( (n = recvmsg(fd, &msg, 0)) < 0) //接受来自客户端的数据,填写msg
		return(n);

	/* indicates no credentials returned 表明现在还没有证书返回 */
	cmsgcredptr->cmcred_ngroups = 0;
	//cmsgcredptr结构体不为null,且msg的辅助数据中有数据
	//说明证书结构体已经被内核填写
	if (cmsgcredptr && msg.msg_controllen > 0) {
		//把辅助数据指针转换成正确cmsghdr结构体,开始访问这个结构体内部,并提取与之关联的辅助数据(证书)
		struct cmsghdr	*cmptr = (struct cmsghdr *) control;

		if (cmptr->cmsg_len < CONTROL_LEN) //检测辅助数据的大小(不包括末尾填充数据)
			err_quit("control length = %d", cmptr->cmsg_len);
		if (cmptr->cmsg_level != SOL_SOCKET) //检测等级
			err_quit("control level != SOL_SOCKET");
		if (cmptr->cmsg_type != SCM_CREDS) //检测类型是否为证书
			err_quit("control type != SCM_CREDS");
		//都没问题就把辅助数据拷贝到用户传参进来的cmsgcredptr结构体
		memcpy(cmsgcredptr, CMSG_DATA(cmptr), sizeof(struct cmsgcred));
	}

	return(n);
}
