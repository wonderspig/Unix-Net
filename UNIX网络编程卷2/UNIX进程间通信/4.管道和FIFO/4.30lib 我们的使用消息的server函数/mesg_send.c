/* include mesg_send */
#include	"mesg.h"

ssize_t
mesg_send(int fd, struct mymesg *mptr)
{
	return(write(fd, mptr, MESGHDRSIZE + mptr->mesg_len)); //向fd中写入mptr这个结构体,写入放入长度是控制头的长度+数据正文的长度
}
/* end mesg_send */

void
Mesg_send(int fd, struct mymesg *mptr)
{
	ssize_t	n;

	if ( (n = mesg_send(fd, mptr)) != mptr->mesg_len)
		err_quit("mesg_send error");
}
