/* include semaphoreh */
	/* 4the fundamental datatype */
typedef struct {
  pthread_mutex_t	sem_mutex;	/* lock to test and set semaphore value 互斥锁 */
  pthread_cond_t	sem_cond;	/* for transition from 0 to nonzero 条件变量 */
  unsigned int		sem_count;	/* the actual semaphore value 本信号量当前值 */
  int				sem_magic;	/* magic number if open 标志:该信号量结构体是否已经初始化 */
} mysem_t;

#define	SEM_MAGIC	0x67458923

#ifdef	SEM_FAILED
#undef	SEM_FAILED
#define	SEM_FAILED	((mysem_t *)(-1))	/* avoid compiler warnings 函数失败的返回值*/
#endif
/* end semaphoreh */

	/* 4our functions */
int		 mysem_close(mysem_t *); //关闭信号量
int		 mysem_getvalue(mysem_t *, int *); //获得信号量的当前值
mysem_t	*mysem_open(const char *, int, ... ); //打开信号量
int		 mysem_post(mysem_t *); //挂出信号量(信号量值+1)
int		 mysem_trywait(mysem_t *); //非阻塞的使用一个信号量(信号量值-1)
int		 mysem_unlink(const char *); //删除这个信号量
int		 mysem_wait(mysem_t *); //阻塞的使用一个信号量(信号量值-1)

	/* 4and the corresponding wrapper functions */
void	 Mysem_close(mysem_t *);
void	 Mysem_getvalue(mysem_t *, int *);
mysem_t	*Mysem_open(const char *, int, ... );
void	 Mysem_post(mysem_t *);
int		 Mysem_trywait(mysem_t *);
void	 Mysem_unlink(const char *);
void	 Mysem_wait(mysem_t *);
