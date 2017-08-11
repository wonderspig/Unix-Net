#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		fd;

	fd = Open("test1.data", O_RDWR | O_CREAT, FILE_MODE); //读写创建的模式打开文件

	Write_lock(fd, 0, SEEK_SET, 0);		/* parent write locks entire file */
	printf("%s: parent has write lock\n", Gf_time());

	if (Fork() == 0) { //创建子进程,再子进程中
			/* 4first child */
		sleep(1);
		printf("%s: first child tries to obtain write lock\n", Gf_time());
		Writew_lock(fd, 0, SEEK_SET, 0);	/* this should block */
		printf("%s: first child obtains write lock\n", Gf_time());
		sleep(2);
		Un_lock(fd, 0, SEEK_SET, 0);
		printf("%s: first child releases write lock\n", Gf_time());
		exit(0);
	}

	if (Fork() == 0) {
			/* 4second child */
		sleep(3);
		printf("%s: second child tries to obtain read lock\n", Gf_time());
		Readw_lock(fd, 0, SEEK_SET, 0);
		printf("%s: second child obtains read lock\n", Gf_time());
		sleep(4);
		Un_lock(fd, 0, SEEK_SET, 0);
		printf("%s: second child releases read lock\n", Gf_time());
		exit(0);
	}

	/* parent */
	sleep(5);
	Un_lock(fd, 0, SEEK_SET, 0);
	printf("%s: parent releases write lock\n", Gf_time());
	exit(0);
}
