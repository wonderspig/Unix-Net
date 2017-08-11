#include	"unpipc.h"

#define	MESGSIZE	256		/* max #bytes per message, incl. null at end */
#define	NMESG		 16		/* max #messages */
//此结构体存放与共享内存区中
struct shmstruct {		/* struct stored in shared memory */
  sem_t	mutex;			/* three Posix memory-based semaphores 二值信号量*/
  sem_t	nempty; //维护空槽位的信号量
  sem_t	nstored; //维护已使用槽位的信号量
  int	nput;			/* index into msgoff[] for next put 放入缓冲区下一个位置的下标*/
  long	noverflow;		/* #overflows by senders 溢出计数器(还想往共享内存区中放入数据,但是共享内存区红已经没有了空槽位,此时溢出计数器+1)*/
  sem_t	noverflowmutex;	/* mutex for noverflow counter 保护溢出计数器的二值信号量*/
  long	msgoff[NMESG];	/* offset in shared memory of each message 每条消息在共享内存区中的偏移量数组(数组的每一个成员都是一条消息的偏移量)*/
  char	msgdata[NMESG * MESGSIZE];	/* the actual messages 存放数据的缓冲区 */
};
