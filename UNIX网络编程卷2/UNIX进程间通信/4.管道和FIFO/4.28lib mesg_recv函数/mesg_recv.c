/* include mesg_recv */
#include	"mesg.h"

ssize_t
mesg_recv(int fd, struct mymesg *mptr)
{
	size_t	len;
	ssize_t	n;

		/* 4read message header first, to get len of data that follows */
	if ( (n = Read(fd, mptr, MESGHDRSIZE)) == 0) //从fd中读取消息(只读取一个结构体mptr的类型成员和数据正文长度成员)
		return(0);		/* end of file */
	else if (n != MESGHDRSIZE) //如果读取的数据长度不符合标准
		err_quit("message header: expected %d, got %d", MESGHDRSIZE, n);

	if ( (len = mptr->mesg_len) > 0) //如果逐句正文中的数据确实存在,那么把数据正文的长度提取出来,复制一份
		if ( (n = Read(fd, mptr->mesg_data, len)) != len) //循环从fd中读取正文数据,直到读取的数据长度等于正文数据长度为止才结束循环
			err_quit("message data: expected %d, got %d", len, n);
	return(len); //返回已经读取的正文的数据长度
}
/* end mesg_recv */

ssize_t
Mesg_recv(int fd, struct mymesg *mptr)
{
	return(mesg_recv(fd, mptr));
}
