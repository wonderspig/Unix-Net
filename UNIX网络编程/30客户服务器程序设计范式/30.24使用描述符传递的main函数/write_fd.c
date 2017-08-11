/* include write_fd */
#include	"unp.h"

ssize_t
write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];
//如果定义了HAVE_MSGHDR_MSG_CONTROL宏,说明使用msg_control版本
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

	cmptr = CMSG_FIRSTHDR(&msg); //获得和msg关联的第一个辅助数据的指针
	//设置辅助数据的大小
	//cmsg_len的值为去掉末尾填充数据的大小
	//(一个int的大小,也就是一个文件描述符的大小)+cmsghdr结构体的大小(不包括结尾填充数据)
	//CMSG_LEN这个宏可以获得这个值
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET; //等级
	cmptr->cmsg_type = SCM_RIGHTS; //辅助数据类型为传送文件描述符
	//通过cmptr获得实际存储数据的指针,强转成int*后,放入已经打开成功的描述符
	*((int *) CMSG_DATA(cmptr)) = sendfd; 

//否则就是accrights版本
#else
	msg.msg_accrights = (caddr_t) &sendfd;
	msg.msg_accrightslen = sizeof(int);
#endif
	//因为本案例是父子进程匿名unix域通信,不需要直到对端地址,所以置空
	msg.msg_name = NULL; 
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr; //一个字节的发送缓冲区放入iov结构体数组
	iov[0].iov_len = nbytes; //这个iov结构体的大小
	msg.msg_iov = iov; //把填写完成iov结构体数组放入msg结构体
	msg.msg_iovlen = 1; //iov结构体数组的元素个数为1

	return(sendmsg(fd, &msg, 0)); //发送给父进程
}
/* end write_fd */

ssize_t
Write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
	ssize_t		n;

	if ( (n = write_fd(fd, ptr, nbytes, sendfd)) < 0)
		err_sys("write_fd error");

	return(n);
}
