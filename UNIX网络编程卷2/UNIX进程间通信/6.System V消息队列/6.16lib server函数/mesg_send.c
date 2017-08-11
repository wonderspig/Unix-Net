/* include mesg_send */
#include	"mesg.h"

ssize_t
mesg_send(int id, struct mymesg *mptr) //参数为消息队列的id和存放消息的数据结构体
{
	return(msgsnd(id, &(mptr->mesg_type), mptr->mesg_len, 0)); //根据消息id,消息类型,消息正文长度来发送消息
}
/* end mesg_send */

void
Mesg_send(int id, struct mymesg *mptr) //mesg_send的包裹函数
{
	ssize_t	n;

	if ( (n = msgsnd(id, &(mptr->mesg_type), mptr->mesg_len, 0)) == -1)
		err_sys("mesg_send error");
}
