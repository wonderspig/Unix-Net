/* include mesg_recv */
#include	"mesg.h"

ssize_t
mesg_recv(int id, struct mymesg *mptr) //函数参数为消息队列id和存放从消息队列中取出来的结构体
{
	ssize_t	n; //用来保存取得消息的正文数据部分的大小
    //从消息队列为id的消息队列中取出消息
    //存放消息缓冲区的指针,应该指向"紧挨着真正的消息数据之前返回的长整数类型字段",所以填入&(mptr->mesg_type)
    //msgrcv函数会自动填入取出消息的消息类型,放在mptr->mesg_type中
    //msgrcv函数会自动填充紧挨着mptr->mesg_type成员之后的真正的data成员,把真正的数据填入
    //MAXMESGDATA是真正的数据长度,不包括mptr->mesg_type字段
    //mptr->mesg_type是要求取出的消息类型(所以结构体mymesg类似与值结果参数)
	n = msgrcv(id, &(mptr->mesg_type), MAXMESGDATA, mptr->mesg_type, 0);
	mptr->mesg_len = n;		/* return #bytes of data 把取出消息的正文部分数据大小,存入mptr->mesg_len中*/

	return(n);				/* -1 on error, 0 at EOF, else >0 返回取出消息的正文部分数据大小*/
}
/* end mesg_recv */
//mesg_recv的包裹函数
ssize_t
Mesg_recv(int id, struct mymesg *mptr)
{
	ssize_t	n;

	if ( (n = mesg_recv(id, mptr)) == -1)
		err_sys("mesg_recv error");
	return(n);
}
