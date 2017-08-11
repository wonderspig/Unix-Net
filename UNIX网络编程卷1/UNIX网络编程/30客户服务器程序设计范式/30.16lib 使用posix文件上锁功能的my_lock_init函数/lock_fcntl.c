/* include my_lock_init */
#include	"unp.h"

static struct flock	lock_it, unlock_it;
static int			lock_fd = -1;
					/* fcntl() will fail if my_lock_init() not called */

void
my_lock_init(char *pathname)
{
    char	lock_file[1024]; //锁文件名

		/* 4must copy caller's string, in case it's a constant */
    strncpy(lock_file, pathname, sizeof(lock_file)); //填写锁文件名(字符串拷贝)
    lock_fd = Mkstemp(lock_file); //根据文件名创建临时文件,并打开这个文件,返回这个临时文件的描述符

    Unlink(lock_file);			/* but lock_fd remains open 解除连接,但文件仍然打开,直到本进程退出,文件会自动删除 */

    //初始化锁结构体

    //加锁结构体
	lock_it.l_type = F_WRLCK; //类型为上写锁
	lock_it.l_whence = SEEK_SET; //起点为文件开始处
	lock_it.l_start = 0; //起点位置是从文件开始处偏移0
	lock_it.l_len = 0; //锁住整个文件
	//解锁结构体
	unlock_it.l_type = F_UNLCK; //类型为解锁
	unlock_it.l_whence = SEEK_SET; //起点为文件开始处
	unlock_it.l_start = 0; //起点位置是从文件开始处偏移0
	unlock_it.l_len = 0; //锁住整个文件
}
/* end my_lock_init */

/* include my_lock_wait */
void
my_lock_wait()
{
    int		rc;
    //对lock_fd这个描述符所代表的文件进行加锁
    while ( (rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0) {
		if (errno == EINTR) //被信号打断则重启系统调用
			continue;
    	else
			err_sys("fcntl error for my_lock_wait");
	}
}

void
my_lock_release()
{
	//对lock_fd这个描述符所代表的文件进行解锁
    if (fcntl(lock_fd, F_SETLKW, &unlock_it) < 0) 
		err_sys("fcntl error for my_lock_release");
}
/* end my_lock_wait */
