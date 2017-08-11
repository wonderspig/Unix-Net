/* include semaphoreh */
	/* 4the fundamental datatype */
typedef struct {
  int	sem_fd[2];	/* two fds: [0] for reading, [1] for writing FIFO的描述符,打开两次,一次用于读,一次用于写*/
  int	sem_magic;	/* magic number if open 标志,表示该结构体是否已经被初始化*/
} mysem_t; //信号量描述符结构体

#define	SEM_MAGIC	0x89674523 //信号量结构体是否被初始化的标志

#ifdef	SEM_FAILED
#undef	SEM_FAILED
#define	SEM_FAILED	((mysem_t *)(-1))	/* avoid compiler warnings 信号量函数错误的返回值 */
#endif
/* end semaphoreh */

	/* 4our functions */
int		 mysem_close(mysem_t *); //关闭信号量
mysem_t	*mysem_open(const char *, int, ... ); //打开信号量(信号量值-1)
int		 mysem_post(mysem_t *); //挂出信号量
int		 mysem_unlink(const char *); //删除信号量
int		 mysem_wait(mysem_t *); //获得一个信号量(信号量值+1)

	/* 4and the corresponding wrapper functions */
void	 Mysem_close(mysem_t *);
mysem_t	*Mysem_open(const char *, int, ... );
void	 Mysem_post(mysem_t *);
void	 Mysem_unlink(const char *);
void	 Mysem_wait(mysem_t *);
