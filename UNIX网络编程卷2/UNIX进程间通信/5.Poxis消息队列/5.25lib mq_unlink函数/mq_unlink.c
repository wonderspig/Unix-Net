/* include mq_unlink */
#include	"unpipc.h"
#include	"mqueue.h"

int
mymq_unlink(const char *pathname)
{
	if (unlink(pathname) == -1) //删除消息队列所对应的映射文件
		return(-1);
	return(0);
}
/* end mq_unlink */

void
Mymq_unlink(const char *pathname)
{
	if (mymq_unlink(pathname) == -1)
		err_sys("mymq_unlink error");
}
