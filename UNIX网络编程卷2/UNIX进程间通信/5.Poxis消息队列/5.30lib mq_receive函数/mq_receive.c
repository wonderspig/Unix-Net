/* include mq_receive1 */
#include	"unpipc.h"
#include	"mqueue.h"

ssize_t
mymq_receive(mymqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop)
{
	int		n;
	long	index;
	int8_t	*mptr;
	ssize_t	len;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymsg_hdr	*msghdr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //消息队列描述符
	if (mqinfo->mqi_magic != MQI_MAGIC) { //如果该消息队列不是正在使用中的,则出错
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr;	/* struct pointer 整个消息队列的控制信息结构体*/
	mptr = (int8_t *) mqhdr;	/* byte pointer mptr指针指向共享内存区的第一个字节 */
	attr = &mqhdr->mqh_attr;    //把本消息队列的属性取出来
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) { //上锁互斥量
		errno = n;
		return(-1);
	}

	if (maxlen < attr->mq_msgsize) { //如果调用者提供的缓冲区长度,比消息的长度小(缓冲区无法存放这个消息),则出错
		errno = EMSGSIZE;
		goto err;
	}
	if (attr->mq_curmsgs == 0) {		/* queue is empty 当前消息队列中没有消息,且指定了非阻塞标志,则出错*/
		if (mqinfo->mqi_flags & O_NONBLOCK) {
			errno = EAGAIN;
			goto err;
		}
			/* 4wait for a message to be placed onto queue 如果没有指定非阻塞标志 */
		mqhdr->mqh_nwait++; //当前阻塞在mymq_receive函数上的线程数+1
		while (attr->mq_curmsgs == 0) //当前消息队列中没有数据
			pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock); //等待条件变量
		mqhdr->mqh_nwait--; //在读取一格数据之前,先把维护阻塞mymq_receive线程的计数器-1
	}
/* end mq_receive1 */
/* include mq_receive2 */

	if ( (index = mqhdr->mqh_head) == 0) //消息队列中从来没有放入过消息(获得所有消息队列链表的第一个消息)
		err_dump("mymq_receive: curmsgs = %ld; head = 0", attr->mq_curmsgs);

	msghdr = (struct mymsg_hdr *) &mptr[index]; //msghdr指针指向第一个消息的控制头结构体
	mqhdr->mqh_head = msghdr->msg_next;	/* new head of list 因为这个消息要取出,所以控制头结构体往后移动一格 */
	len = msghdr->msg_len; //这条消息的正文长度
	memcpy(ptr, msghdr + 1, len);		/* copy the message itself 拷贝这条消息到调用者提供的缓冲区中 */
	if (priop != NULL) //如果这条消息的优先级不为null
		*priop = msghdr->msg_prio; //取出这条消息的优先级

		/* 4just-read message goes to front of free list */
	msghdr->msg_next = mqhdr->mqh_free; //这条消息的下一条消息是空闲消息
	mqhdr->mqh_free = index; //把当前消息置为空闲消息(当前消息已经从消息队列中取出了)

		/* 4wake up anyone blocked in mq_send waiting for room */
	if (attr->mq_curmsgs == attr->mq_maxmsg) //如果当前消息队列中的消息数量,已经是最大数量了
		pthread_cond_signal(&mqhdr->mqh_wait); //唤醒阻塞在条件变量中的线程
	attr->mq_curmsgs--; //当前消息数量-1

	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(len);

err:
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(-1);
}
/* end mq_receive2 */

ssize_t
Mymq_receive(mymqd_t mqd, char *ptr, size_t len, unsigned int *priop)
{
	ssize_t	n;

	if ( (n = mymq_receive(mqd, ptr, len, priop)) == -1)
		err_sys("mymq_receive error");
	return(n);
}
