#include	"unpipc.h"

/* Our own "messages" to use with pipes, FIFOs, and message queues. */

	/* 4want sizeof(struct mymesg) <= PIPE_BUF */
#define	MAXMESGDATA	(PIPE_BUF - 2*sizeof(long)) //消息的最大长度,保证整个mesg结构体的长度不会大于PIPE_BUF,保证一次write是原子的

	/* 4length of mesg_len and mesg_type */
#define	MESGHDRSIZE	(sizeof(struct mymesg) - MAXMESGDATA) //单个消息的控制信息长度

struct mymesg {
  long	mesg_len;	/* #bytes in mesg_data, can be 0 消息数据正文的长度*/
  long	mesg_type;	/* message type, must be > 0 消息的类型*/
  char	mesg_data[MAXMESGDATA]; //消息的正文数据
};

ssize_t	 mesg_send(int, struct mymesg *); //发送消息的函数
void	 Mesg_send(int, struct mymesg *);
ssize_t	 mesg_recv(int, struct mymesg *); //接受消息的函数
ssize_t	 Mesg_recv(int, struct mymesg *);
