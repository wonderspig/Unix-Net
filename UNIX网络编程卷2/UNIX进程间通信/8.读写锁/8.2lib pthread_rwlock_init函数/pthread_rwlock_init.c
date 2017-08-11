/* include init */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
pthread_rwlock_init(pthread_rwlock_t *rw, pthread_rwlockattr_t *attr)
{
	int		result;

	if (attr != NULL) //不支持给读写锁附加属性
		return(EINVAL);		/* not supported */

	if ( (result = pthread_mutex_init(&rw->rw_mutex, NULL)) != 0) //初始化互斥量
		goto err1;
	if ( (result = pthread_cond_init(&rw->rw_condreaders, NULL)) != 0) //初始化读条件变量
		goto err2;
	if ( (result = pthread_cond_init(&rw->rw_condwriters, NULL)) != 0) //初始化写条件变量
		goto err3;
	rw->rw_nwaitreaders = 0; //为读而阻塞的线程初始化为0
	rw->rw_nwaitwriters = 0; //为写而阻塞的线程初始化0
	rw->rw_refcount = 0; //该读写锁现在所处的状态,初始化为0
	rw->rw_magic = RW_MAGIC; //设置标志,表示该读写锁初始化完毕

	return(0);

err3:
	pthread_cond_destroy(&rw->rw_condreaders);
err2:
	pthread_mutex_destroy(&rw->rw_mutex);
err1:
	return(result);			/* an errno value */
}
/* end init */

void
Pthread_rwlock_init(pthread_rwlock_t *rw, pthread_rwlockattr_t *attr)
{
	int		n;

	if ( (n = pthread_rwlock_init(rw, attr)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_init error");
}
