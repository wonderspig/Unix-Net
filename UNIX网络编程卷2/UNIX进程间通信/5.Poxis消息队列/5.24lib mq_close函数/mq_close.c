/* include mq_close */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_close(mymqd_t mqd)
{
	long	msgsize, filesize;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd; //把传参进来的消息队列描述符复制一份
	if (mqinfo->mqi_magic != MQI_MAGIC) { //如果这个消息队列不是正在使用的,则出错
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr; //整个消息队列链表的控制信息结构体
	attr = &mqhdr->mqh_attr; //消息队列的属性

	if (mymq_notify(mqd, NULL) != 0)	/* unregister calling process 删除消息队列非空是同志的进程(取消注册通知的进程) */
		return(-1);

	msgsize = MSGSIZE(attr->mq_msgsize); //每条消息队列的最大大小,向上取整为sizeof(long)的倍数
	//文件的大小等于文件开头结构体mymq_hdr(用来控制整个消息队列链表的结构体)+最大消息数目*每条消息的大小
	//其中,每条消息大小=mymsg_hdr(每条消息开头的控制结构体)+消息正文本身的大小(以向上取整为sizeof(long)的倍数)
	filesize = sizeof(struct mymq_hdr) + (attr->mq_maxmsg *
			   (sizeof(struct mymsg_hdr) + msgsize)); 
	if (munmap(mqinfo->mqi_hdr, filesize) == -1) //用初始化哈的文件大小解除映射
		return(-1);

	mqinfo->mqi_magic = 0;		/* just in case 关闭的消息队列描述符结构体取消已经使用的标志 */
	free(mqinfo); //释放动态分配的消息队列描述符结构体
	return(0); //成功返回0
}
/* end mq_close */

void
Mymq_close(mymqd_t mqd)
{
	if (mymq_close(mqd) == -1)
		err_sys("mymq_close error");
}
