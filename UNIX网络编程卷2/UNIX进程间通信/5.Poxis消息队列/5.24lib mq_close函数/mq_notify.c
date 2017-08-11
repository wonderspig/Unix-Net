/* include mq_notify */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_notify(mymqd_t mqd, const struct sigevent *notification)
{
	int		n;
	pid_t	pid;
	struct mymq_hdr	*mqhdr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //消息队列结构体
	if (mqinfo->mqi_magic != MQI_MAGIC) { //如果这个消息队列并非使用中的消息队列,则出错
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr; //整个消息队列的控制头结构体
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) { //加锁互斥量
		errno = n;
		return(-1);
	}

	pid = getpid(); //获得当前进程的pid
	if (notification == NULL) { //调用者要求注册进程不接受通知
		if (mqhdr->mqh_pid == pid) { //当前进程正是接收通知的进程
			mqhdr->mqh_pid = 0;	/* unregister calling process 取消注册接收通知的进程*/
		} 						/* no error if caller not registered 奇怪的是如果当前进程并没有被注册为接收通知的进程,本函数并不报错*/
	} else { //调用者进程要求注册接受通知,但是之前已经有别的进程注册过
		if (mqhdr->mqh_pid != 0) { //有被注册接收通知的进程,但并不是调用者进程
			//向原先注册的接收通知的进程发送一个空信号,如果返回ESRCH错误,那么表示这个进程已经不存在
			//原先的进程仍然存在,则返回调用者EBUSY错误(如果原先注册的进程不存在,会返回ESRCH错误)
			if (kill(mqhdr->mqh_pid, 0) != -1 || errno != ESRCH) { 
				errno = EBUSY; //我们返回调用者一个EBUSY错误
				goto err;
			}
		}
		mqhdr->mqh_pid = pid; //重新设置注册进程
		mqhdr->mqh_event = *notification; //把注册进程信息的结构体填入"整个消息队列的控制头结构体的相应成员之中"
	}
	pthread_mutex_unlock(&mqhdr->mqh_lock); //解锁互斥量
	return(0);

err:
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(-1);
}
/* end mq_notify */

void
Mymq_notify(mymqd_t mqd, const struct sigevent *notification)
{
	if (mymq_notify(mqd, notification) == -1)
		err_sys("mymq_notify error");
}
