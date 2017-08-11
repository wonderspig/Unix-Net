/* include mq_getattr */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_getattr(mymqd_t mqd, struct mymq_attr *mqstat)
{
	int		n;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //消息队列描述符结构体
	if (mqinfo->mqi_magic != MQI_MAGIC) { //如果此消息队列不是正在使用的,则出错
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr; //整个消息队列链表的控制信息结构体
	attr = &mqhdr->mqh_attr; //消息队列的属性
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) { //锁上这个消息队列
		errno = n; //出错则设置errno的值
		return(-1);
	}
	//填写消息队列属性
	mqstat->mq_flags = mqinfo->mqi_flags;	/* per-open 填写标志*是否阻塞 */
	mqstat->mq_maxmsg = attr->mq_maxmsg;	/* remaining three per-queue 消息队列的最大消息数 */
	mqstat->mq_msgsize = attr->mq_msgsize; //每条消息的最大大小
	mqstat->mq_curmsgs = attr->mq_curmsgs; //当前消息队列中的消息数

	pthread_mutex_unlock(&mqhdr->mqh_lock); //解锁互斥量
	return(0); //正确返回0
}
/* end mq_getattr */

void
Mymq_getattr(mymqd_t mqd, struct mymq_attr *mqstat)
{
	if (mymq_getattr(mqd, mqstat) == -1)
		err_sys("mymq_getattr error");
}
