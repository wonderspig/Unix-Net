/* include rwlockh */
#ifndef	__pthread_rwlock_h
#define	__pthread_rwlock_h

typedef struct {
  pthread_mutex_t	rw_mutex;		/* basic lock on this struct 无论何时检查或操作该结构,都必须持有这个互斥锁 */
  pthread_cond_t	rw_condreaders;	/* for reader threads waiting 读锁条件变量 */
  pthread_cond_t	rw_condwriters;	/* for writer threads waiting 写锁条件变量 */
  int				rw_magic;		/* for error checking 标志:RW_MAGIC,表示这个读写锁有效,所有函数都测试这个成员 */
  int				rw_nwaitreaders;/* the number waiting 阻塞等待着的读线程 */
  int				rw_nwaitwriters;/* the number waiting 阻塞等待着的写线程 */
  int				rw_refcount; //读写锁的当前状态:-1为写锁,0为该读写锁当前可用,大于0表示当前容纳着这么多的读锁
	/* 4-1 if writer has the lock, else # readers holding the lock */
} pthread_rwlock_t;

#define	RW_MAGIC	0x19283746 //标志当前读写锁是已经初始化过的

	/* 4following must have same order as elements in struct above */
#define	PTHREAD_RWLOCK_INITIALIZER	{ PTHREAD_MUTEX_INITIALIZER, \
			PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, \
			RW_MAGIC, 0, 0, 0 } //读写锁结构体的初始化值 

typedef	int		pthread_rwlockattr_t;	/* dummy; not supported 不支持*/

	/* 4function prototypes */
int		pthread_rwlock_destroy(pthread_rwlock_t *); //销毁读写锁
int		pthread_rwlock_init(pthread_rwlock_t *, pthread_rwlockattr_t *); //初始化读写锁
int		pthread_rwlock_rdlock(pthread_rwlock_t *); //加读锁
int		pthread_rwlock_tryrdlock(pthread_rwlock_t *); //非阻塞加读锁
int		pthread_rwlock_trywrlock(pthread_rwlock_t *); //非阻塞加写锁
int		pthread_rwlock_unlock(pthread_rwlock_t *); //解除读写锁
int		pthread_rwlock_wrlock(pthread_rwlock_t *); //加写锁
/* $$.bp$$ */
	/* 4and our wrapper functions */
void	Pthread_rwlock_destroy(pthread_rwlock_t *);
void	Pthread_rwlock_init(pthread_rwlock_t *, pthread_rwlockattr_t *);
void	Pthread_rwlock_rdlock(pthread_rwlock_t *);
int		Pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int		Pthread_rwlock_trywrlock(pthread_rwlock_t *);
void	Pthread_rwlock_unlock(pthread_rwlock_t *);
void	Pthread_rwlock_wrlock(pthread_rwlock_t *);

#endif	__pthread_rwlock_h
/* end rwlockh */
