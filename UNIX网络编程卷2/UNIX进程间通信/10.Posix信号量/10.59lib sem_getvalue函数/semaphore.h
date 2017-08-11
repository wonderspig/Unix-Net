/* include semaphoreh */
	/* 4the fundamental datatype */
typedef struct {
  int	sem_semid;		/* the System V semaphore ID System V信号量ID*/
  int	sem_magic;		/* magic number if open 魔数,每个mysem_t指针都检查该值,确定信号量是否初始化*/
} mysem_t;

#define	SEM_MAGIC	0x45678923

#ifdef	SEM_FAILED
#undef	SEM_FAILED
#define	SEM_FAILED	((mysem_t *)(-1))	/* avoid compiler warnings */
#endif

#ifndef	SEMVMX
#define	SEMVMX	32767		/* historical System V max value for sem 信号量值的最大值 */
#endif
/* end semaphoreh */

	/* 4our functions */
int		 mysem_close(mysem_t *); //关闭信号量
int		 mysem_getvalue(mysem_t *, int *); //获得信号量的值
mysem_t	*mysem_open(const char *, int, ... ); //打开信号量
int		 mysem_post(mysem_t *); //释放信号量,信号量的值+1
int		 mysem_trywait(mysem_t *); //使用一个信号量,信号量值-1,如果当前的信号量值为0,则不阻塞,报错返回
int		 mysem_unlink(const char *); //删除一个信号量
int		 mysem_wait(mysem_t *); ///使用一个信号量,信号量值-1,如果当前的信号量值为0,则阻塞

	/* 4and the corresponding wrapper functions */
void	 Mysem_close(mysem_t *);
void	 Mysem_getvalue(mysem_t *, int *);
mysem_t	*Mysem_open(const char *, int, ... );
void	 Mysem_post(mysem_t *);
int		 Mysem_trywait(mysem_t *);
void	 Mysem_unlink(const char *);
void	 Mysem_wait(mysem_t *);
