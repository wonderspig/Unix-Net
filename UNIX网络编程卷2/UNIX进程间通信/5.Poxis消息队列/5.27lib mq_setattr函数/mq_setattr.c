/* include mq_setattr */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_setattr(mymqd_t mqd, const struct mymq_attr *mqstat,
			 struct mymq_attr *omqstat)
{
	int		n;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //消息队列描述符
	if (mqinfo->mqi_magic != MQI_MAGIC) {
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr; //整个消息队列链表的控制信息结构体
	attr = &mqhdr->mqh_attr; //消息队列的属性
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) { //加锁互斥量
		errno = n;
		return(-1);
	}
	//调用者传参进来的用来存放设置之前的消息队列属性的结构体非空
	if (omqstat != NULL) { 
		omqstat->mq_flags = mqinfo->mqi_flags;	/* previous attributes 消息队列的标志(是否阻塞)*/
		omqstat->mq_maxmsg = attr->mq_maxmsg; //消息队列最大的消息数量
		omqstat->mq_msgsize = attr->mq_msgsize; //每个消息的最大大小
		omqstat->mq_curmsgs = attr->mq_curmsgs;	/* and current status 消息队列中当前的消息大小 */
	}

	if (mqstat->mq_flags & O_NONBLOCK) //传参进来的设置值中要求非阻塞
		mqinfo->mqi_flags |= O_NONBLOCK; //则加入非阻塞标志
	else
		mqinfo->mqi_flags &= ~O_NONBLOCK; //否则删除非阻塞标志

	pthread_mutex_unlock(&mqhdr->mqh_lock); //解锁互斥量
	return(0);
}
/* end mq_setattr */

void
Mymq_setattr(mymqd_t mqd, const struct mymq_attr *mqstat,
			 struct mymq_attr *omqstat)
{
	if (mymq_setattr(mqd, mqstat, omqstat) == -1)
		err_sys("mymq_setattr error");
}
