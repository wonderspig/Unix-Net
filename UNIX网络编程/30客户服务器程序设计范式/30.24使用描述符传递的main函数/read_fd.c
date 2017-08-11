/* include read_fd */
#include	"unp.h"

ssize_t
read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	ssize_t			n;

//如果定义了HAVE_MSGHDR_MSG_CONTROL宏,代表使用的是msg_control版本
#ifdef	HAVE_MSGHDR_MSG_CONTROL 
	union {
	  struct cmsghdr	cm;
	  //CMSG_SPACE宏计算的是加上末尾填充字节的整个辅助数据对象的大小
	  //这是为了保证结构体对齐,联合的大小是里面最大的一个成员的大小
	  //这就保证了联合最小也有CMSG_SPACE(sizeof(int))这个大小
	  //CMSG_SPACE保证了足够存放下一个辅助数据对象的大小
	  char				control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	//给msg结构体的辅助数据进行附值
	msg.msg_control = control_un.control; //对指针进行附值
	msg.msg_controllen = sizeof(control_un.control); //指针指向的地址空间有足够的大小

//否则就是accrights版本
#else
	int				newfd;

	msg.msg_accrights = (caddr_t) &newfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	//因为本案例是父子进程匿名unix域通信,不需要直到对端地址,所以置空
	msg.msg_name = NULL; 
	msg.msg_namelen = 0;


	iov[0].iov_base = ptr; //把传参进来的接受缓冲区填入iov结构体数组
	iov[0].iov_len = nbytes; //接受缓冲区的大小
	msg.msg_iov = iov; //把设置好的iov结构体数组放入msg结构体
	msg.msg_iovlen = 1; //iov结构体数组的成员树木

	//用填写完毕的msg结构体调用recvmsg,把结果存放在msg中
	if ( (n = recvmsg(fd, &msg, 0)) <= 0) 
		return(n);

//如果定义了HAVE_MSGHDR_MSG_CONTROL宏,代表使用的是msg_control版本
#ifdef	HAVE_MSGHDR_MSG_CONTROL 
	//检查msg结构体中,存放的所有辅助数据

	//msg所关联的第一个辅助数据的指针不为null(辅助数据确实存在)
	//第一个辅助数据指针指向的大小cmsg_len的值(去掉结尾填充数据的大小)
	//应该等于辅助数据去掉末尾填充数据的大小
	//(一个int的大小,也就是一个文件描述符的大小)+cmsghdr结构体的大小(不包括结尾填充数据)
	//以上条件成立代表确实存在一个大小符合要求的辅助数据
	if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
	    cmptr->cmsg_len == CMSG_LEN(sizeof(int))) { 
		if (cmptr->cmsg_level != SOL_SOCKET) //查看这个辅助数据的等级
			err_quit("control level != SOL_SOCKET");
		if (cmptr->cmsg_type != SCM_RIGHTS) //辅助数据的类型是否为文件描述符
			err_quit("control type != SCM_RIGHTS");
		//CMSG_DATA宏可以提取辅助数据的具体内容(也就是描述符)
		*recvfd = *((int *) CMSG_DATA(cmptr)); //把描述符存放到recvfd中,用来返回给调用者
	} //else否则返回-1代表出错
		*recvfd = -1;		/* descriptor was not passed */

//否则就是accrights版本
#else
/* *INDENT-OFF* */
	if (msg.msg_accrightslen == sizeof(int))
		*recvfd = newfd;
	else
		*recvfd = -1;		/* descriptor was not passed */
/* *INDENT-ON* */
#endif

	return(n);
}
/* end read_fd */

ssize_t
Read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
	ssize_t		n;

	if ( (n = read_fd(fd, ptr, nbytes, recvfd)) < 0)
		err_sys("read_fd error");

	return(n);
}
