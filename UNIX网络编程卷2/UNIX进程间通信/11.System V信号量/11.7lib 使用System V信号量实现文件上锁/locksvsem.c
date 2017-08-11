//Posix信号量中有一段文件上锁的函数，10.19中的posix文件上锁也可以用如下的System V信号量的文件上锁来实现
#include	"unpipc.h"

#define	LOCK_PATH	"/tmp/svsemlock"
#define	MAX_TRIES	10

int		semid, initflag;
struct sembuf	postop, waitop;

void
my_lock(int fd)
{
	int		oflag, i;
	union semun	arg;
	struct semid_ds	seminfo;

	if (initflag == 0) { //初始化标志为0(全局变量在声明时自动初始化为0)
		oflag = IPC_CREAT | IPC_EXCL | 644; //排他创建信号量
		//排他创建信号量,使用LOCK_PATH创建key,信号量集中的信号量个数为1个
		if ( (semid = semget(Ftok(LOCK_PATH, 0), 1, oflag)) >= 0) {
			//创建成功就进入此分支
				/* 4success, we're the first so initialize */
			arg.val = 1; //信号量的值设置为1
			Semctl(semid, 0, SETVAL, arg); //设置信号量的值

		} else if (errno == EEXIST) { //进入此分支代表这个路径下的信号量已存在
				/* 4someone else has created; make sure it's initialized */
			semid = Semget(Ftok(LOCK_PATH, 0), 1, 644); //那就直接打开此信号量
			arg.buf = &seminfo; //信号量的状态结构体指针
			//用循环检查信号量状态中的sem_otime值是否变为当前时间
			//System V的信号量的初始化分为创建和初始化两步,非原子性,所以存在竞争状态
			//当创建进程创建完信号量,并对它进行初始化,再调用semop对信号量进行操作之后,sem_otime才会被设置成当前值
			//此时其他进程才能使用这个信号量
			for (i = 0; i < MAX_TRIES; i++) { //以10次为限,检查sem_otime的值
				Semctl(semid, 0, IPC_STAT, arg); //获得当前信号量集的状态
				if (arg.buf->sem_otime != 0) //如果sem_otime还没有变成非0
					goto init; //跳转到init
				sleep(1); //如果sem_otime为0,则睡一秒继续循环尝试
			}
			//如果实验了10次还没有等待到sem_otime非0,则报错
			err_quit("semget OK, but semaphore not initialized");

		} else //非EEXIST这个错误,导致semget的失败
			err_sys("semget error");
init: //此时sem_otime已经非零,本进程已经可以对信号量进行操作
		initflag = 1; //标识量置为1,表示不再需要信号量的创建或者初始化,也不需要等待信号量的初始化完成
		//释放信号量的结构体
		postop.sem_num = 0;		/* and init the two semop() structures 操作第一个信号量的结构体 */
		postop.sem_op  = 1; //给信号量的值+1
		postop.sem_flg = SEM_UNDO; //此标志表示进程结束后信号量值会恢复
		waitop.sem_num = 0; //操作第一个信号量的结构体
		waitop.sem_op  = -1; //给信号量的值-1
		waitop.sem_flg = SEM_UNDO; //此标志表示进程结束后信号量值会恢复
	}
	Semop(semid, &waitop, 1);		/* down by 1 使用waitop,给信号量的值-1*/
}

void
my_unlock(int fd) //解锁信号量
{
	Semop(semid, &postop, 1);		/* up by 1 给信号量的值+1*/
}
