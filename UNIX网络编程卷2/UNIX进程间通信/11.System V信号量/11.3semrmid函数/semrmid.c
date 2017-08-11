#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		semid;

	if (argc != 2) //如果命令行参数不等于1,则报错
		err_quit("usage: semrmid <pathname>");

	semid = Semget(Ftok(argv[1], 0), 0, 0); //用命令行的第一个参数创建key,来打开已存在的信号量
	Semctl(semid, 0, IPC_RMID); //删除这个信号量

	exit(0);
}
