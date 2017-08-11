/* include mqueueh */
typedef struct mymq_info	*mymqd_t;		/* opaque datatype 每次调用mq_open返回一个mymq_info的指针*/
  //描述消息队列属性的结构体
struct mymq_attr {
  long	mq_flags;		/* message queue flag: O_NONBLOCK 唯一的标志是O_NONBLOCK,此标志在此处不维护,
                        在mymq_info这个结构体中维护,只有每次打开消息队列的时候,才能指定是否为非阻塞模式 */
  long	mq_maxmsg;		/* max number of messages allowed on queue 最大的消息数量 */
  long	mq_msgsize;		/* max size of a message (in bytes) 每个消息的最大大小 */
  long	mq_curmsgs;		/* number of messages currently on queue 当前的消息数量 */
};

	/* 4one mymq_hdr{} per queue, at beginning of mapped file */
//此结构体维护整个内存映射文件
struct mymq_hdr {
  struct mymq_attr	mqh_attr;	/* the queue's attributes 消息队列的属性结构体 */
  long				mqh_head;	/* index of first message 当前队列中的所有消息 */
  long				mqh_free;	/* index of first free message 当前队列中所有的空闲消息 */
  long				mqh_nwait;	/* #threads blocked in mq_receive() 接收消息时阻塞的线程 */
  pid_t				mqh_pid;	/* nonzero PID if mqh_event set 如果设置消息队列非空就通知进程,此处为待通知的进程pid */
  struct sigevent	mqh_event;	/* for mq_notify() 消息队列非空通知进程 */
  pthread_mutex_t	mqh_lock;	/* mutex lock 互斥锁 */
  pthread_cond_t	mqh_wait;	/* and condition variable 条件变量 */
};

	/* 4one mymsg_hdr{} at the front of each message in the mapped file */
//每条消息的控制头
struct mymsg_hdr {
  //消息要么在已使用消息的链表中,要么在空闲的链表中
  long	msg_next;				/* index of next on linked list 本成员含有的链表中下一条消息的指针,本消息时最后一条消息,则为0 */
								/* 4msg_next must be first member in struct */
  ssize_t	msg_len;			/* actual length 正文消息数据的长度 */
  unsigned int	msg_prio;		/* priority 本条消息的优先级 */
};

	/* 4one mymq_info{} malloc'ed per process per mq_open() 每次调用mq_open返回一个mymq_info的指针 */
struct mymq_info {
  struct mymq_hdr	*mqi_hdr;	/* start of mmap'ed region 指向映射文件(mmap返回的起始地址) */
  long	mqi_magic;				/* magic number if open 本结构一旦初始化,本成员为MQI_MAGIC,
                              一调用mymq_open,就检查这个成员,以验证是否指向mymq_info结构体 */
  int	mqi_flags;				/* flags for this process 是否设置非阻塞标志*/
};
#define	MQI_MAGIC	0x98765432 //用来设置mymq_info结构体的mqi_magic成员

	/* 4size of message in file is rounded up for alignment */
//为了对齐,我们希望映射文件中的每条消息从一个长整数边界开始
//此宏前半部分(((i) + sizeof(long)-1) / sizeof(long))的值在除不禁的情况下会自动取整,假设sizeof(long)＝8
//如小于8除下来就是1,8~16之间的数除下来就是2,16~32之间的数除下来就是3,依此类推
//当这个整数去乘以8的时候,就保证了一定是向上取整的8的倍数,也就是sizeof(long)的倍数,达到了对齐长整型边界的效果
#define	MSGSIZE(i)	((((i) + sizeof(long)-1) / sizeof(long)) * sizeof(long))
/* end mqueueh */

	/* 4our functions 操作消息队列的函数 */
int		 mymq_close(mymqd_t); //关闭消息队列
int		 mymq_getattr(mymqd_t, struct mymq_attr *); //获得消息队列的属性
int		 mymq_notify(mymqd_t, const struct sigevent *); //注册当前进程接收消息队列的通知
mymqd_t	 mymq_open(const char *, int, ...); //打开消息队列
ssize_t	 mymq_receive(mymqd_t, char *, size_t, unsigned int *); //读取消息队列中的消息
int		 mymq_send(mymqd_t, const char *, size_t, unsigned int); //向喜爱队列中发送数据
int		 mymq_setattr(mymqd_t, const struct mymq_attr *, struct mymq_attr *); //设置消息队列属性
int		 mymq_unlink(const char *name); //删除该消息队列

	/* 4and the corresponding wrapper functions */
void	 Mymq_close(mymqd_t);
void	 Mymq_getattr(mymqd_t, struct mymq_attr *);
void	 Mymq_notify(mymqd_t, const struct sigevent *);
mymqd_t	 Mymq_open(const char *, int, ...);
ssize_t	 Mymq_receive(mymqd_t, char *, size_t, unsigned int *);
void	 Mymq_send(mymqd_t, const char *, size_t, unsigned int);
void	 Mymq_setattr(mymqd_t, const struct mymq_attr *, struct mymq_attr *);
void	 Mymq_unlink(const char *name);
