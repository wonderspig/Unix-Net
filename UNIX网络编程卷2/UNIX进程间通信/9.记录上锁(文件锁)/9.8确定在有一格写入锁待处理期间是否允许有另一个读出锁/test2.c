#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		fd;

	fd = Open("test1.data", O_RDWR | O_CREAT, FILE_MODE); //以读写创建的方式打开文件

	Read_lock(fd, 0, SEEK_SET, 0);		/* parent read locks entire file 非阻塞的获取读锁 */
	printf("%s: parent has read lock\n", Gf_time()); //打印父进程获得读锁的时间

	if (Fork() == 0) { //创建子进程,再子进程中
			/* 4first child */
		sleep(1); //睡一秒
		printf("%s: first child tries to obtain write lock\n", Gf_time()); //第一个子进程开始获得写锁
		Writew_lock(fd, 0, SEEK_SET, 0);	/* this should block 获取写锁,阻塞*/
		printf("%s: first child obtains write lock\n", Gf_time()); //第一个子进程获得了写锁
		sleep(2); //睡两秒
		Un_lock(fd, 0, SEEK_SET, 0); //解锁
		printf("%s: first child releases write lock\n", Gf_time()); //第一个子进程释放了写锁
		exit(0);
	}
	//父进程创建第二个子进程
	if (Fork() == 0) { //再子进程中
			/* 4second child */
		sleep(3); //睡3秒
		printf("%s: second child tries to obtain read lock\n", Gf_time()); //第二个子进程开始获取读锁
		Readw_lock(fd, 0, SEEK_SET, 0); //获取读锁
		printf("%s: second child obtains read lock\n", Gf_time()); //第二个子进程获取了读锁
		sleep(4); //睡4秒
		Un_lock(fd, 0, SEEK_SET, 0); //解锁
		printf("%s: second child releases read lock\n", Gf_time()); //第二个子进程释放读锁
		exit(0);
	}

	/* 4parent */
	sleep(5); //父进程睡5秒
	Un_lock(fd, 0, SEEK_SET, 0); //释放读锁
	printf("%s: parent releases read lock\n", Gf_time()); //打印父进程释放了读锁
	exit(0);
}
