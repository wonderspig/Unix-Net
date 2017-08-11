/* include mq_send1 */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_send(mymqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	int		n;
	long	index, freeindex;
	int8_t	*mptr;
	struct sigevent	*sigev;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymsg_hdr	*msghdr, *nmsghdr, *pmsghdr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //消息队列描述符
	if (mqinfo->mqi_magic != MQI_MAGIC) { //如果该消息队列未使用,则报错
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr;	/* struct pointer 整个消息队列的控制结构体*/
	mptr = (int8_t *) mqhdr;	/* byte pointer "整个消息队列的控制结构体"位于映射文件的开头,所以这样附值合理 */
	attr = &mqhdr->mqh_attr; //"整个消息队列的控制结构体"中的"消息队列属性成员"
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) { //加锁互斥量
		errno = n;
		return(-1);
	}

	if (len > attr->mq_msgsize) { //调用者指定的消息长度,大于每条消息的最大长度,则出错
		errno = EMSGSIZE;
		goto err;
	}
	if (attr->mq_curmsgs == 0) { //当前消息队列中的消息数量为0
		if (mqhdr->mqh_pid != 0 && mqhdr->mqh_nwait == 0) { //有注册进程接收消息通知,没有进程阻塞于mq_receive调用中
			sigev = &mqhdr->mqh_event; //获得具体的注册信息
			if (sigev->sigev_notify == SIGEV_SIGNAL) { //如果是信号通知方式
				sigqueue(mqhdr->mqh_pid, sigev->sigev_signo,
						 sigev->sigev_value); //发送可排队的可靠实时信号,发送给mqhdr->mqh_pid,发送sigev->sigev_signo信号
											  //随信号的附加数据为sigev->sigev_value
			}
			mqhdr->mqh_pid = 0;		/* unregister 取消注册的进程 */
		}
	} else if (attr->mq_curmsgs >= attr->mq_maxmsg) { //当前队列的消息数量已经大于最大消息数量
			/* 4queue is full */
		if (mqinfo->mqi_flags & O_NONBLOCK) { //如果有非阻塞标志,则出错
			errno = EAGAIN;
			goto err;
		}
			/* 4wait for room for one message on the queue */
		while (attr->mq_curmsgs >= attr->mq_maxmsg) //否则用条件变量陷入睡眠,进行等待
			pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);
	}
/* end mq_send1 */
/* include mq_send2 */
		/* 4nmsghdr will point to new message */
	if ( (freeindex = mqhdr->mqh_free) == 0) //获得指向空闲链表的第一个消息的指针索引
		err_dump("mymq_send: curmsgs = %ld; free = 0", attr->mq_curmsgs);
	nmsghdr = (struct mymsg_hdr *) &mptr[freeindex]; //第一条空闲消息的控制信息结构体指针
	nmsghdr->msg_prio = prio; //消息的等级
	nmsghdr->msg_len = len; //消息的正文长度
	memcpy(nmsghdr + 1, ptr, len);		/* copy message from caller 把消息正文放入消息队列中*/
	mqhdr->mqh_free = nmsghdr->msg_next;	/* new freelist head 空闲链表的指针指向下一个空闲消息
												(在mq_open中,已经初始化了所有消息的msg_next的位置)*/

		/* 4find right place for message in linked list */
	//链表中各个消息的顺序是从开始处(mq_head)的最高优先级到结束处的最低优先级

	index = mqhdr->mqh_head; //获得当前的消息队列中第一个消息的指针索引
	//pmsghdr保存的是上一个消息的指针索引,因为它的msg_next值保存当前消息的索引
	pmsghdr = (struct mymsg_hdr *) &(mqhdr->mqh_head); //获得消息队列中第一个消息的消息控制头结构体(pmsghdr从未出现在等号右边)
	while (index != 0) { //循环遍历查找调用者需要插入的消息,在消息队列中应该存放的位置
		//当一个新消息加入调用者指定的队列中,并且一个或多个同样优先级的消息已在该队列中时,这个新消息就加在最后一个优先级相同的消息之后
		msghdr = (struct mymsg_hdr *) &mptr[index]; //消息链表的第一个消息的控制头结构体
		if (prio > msghdr->msg_prio) { 
			nmsghdr->msg_next = index; 
			pmsghdr->msg_next = freeindex;
			break;
		}
		index = msghdr->msg_next; //消息链表往后走一格
		pmsghdr = msghdr; //pmsghdr为上一个消息的索引,msghdr在循环一开始就要调用往后走了一格的index,所以在往后走之前先附值给pmsghdr
	}
	if (index == 0) { //当前消息队列中没有消息
			/* 4queue was empty or new goes at end of list */
		pmsghdr->msg_next = freeindex; //pmsghdr为上一个消息的索引,因为它的msg_next值保存当前消息的索引(第一个空闲链表的指针索引)
		nmsghdr->msg_next = 0; //当前消息的msg_next值为0(当前消息队列中没有消息,所以第一个插进去的消息的下一个值自然为0)
	}
		/* 4wake up anyone blocked in mq_receive waiting for a message */
	if (attr->mq_curmsgs == 0) //如果当前消息队列中没有消息
		pthread_cond_signal(&mqhdr->mqh_wait); //唤醒阻塞于取消息的进程
	attr->mq_curmsgs++; //当前消息队列中的消息数目+1

	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(0);

err:
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(-1);
}
/* end mq_send2 */

void
Mymq_send(mymqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	if (mymq_send(mqd, ptr, len, prio) == -1)
		err_sys("mymq_send error");
}
