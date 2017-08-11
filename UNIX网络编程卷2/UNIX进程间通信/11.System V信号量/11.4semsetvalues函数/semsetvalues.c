#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		semid, nsems, i;
	struct semid_ds	seminfo;
	unsigned short	*ptr;
	union semun	arg;

	if (argc < 2) //如果命令行参数小于1,则出错
		err_quit("usage: semsetvalues <pathname> [ values ... ]");

		/* 4first get the number of semaphores in the set */
	semid = Semget(Ftok(argv[1], 0), 0, 0); //用命令行的第一个参数创建key,来打开已存在的信号量
	arg.buf = &seminfo; //arg.buf为semid_ds结构体,获得当前信号量集的当前状态
	Semctl(semid, 0, IPC_STAT, arg); //用Semctl函数获得当前信号量状态,填充semid_ds结构体
	nsems = arg.buf->sem_nsems; //获得信号量集当前状态中的"信号量数组的个数"信息

		/* 4now get the values from the command line */
	//命令行参数中,第一个参数为程序运行名,第二个参数为路径名,第三个参数开始是设置信号量的值
	//所以命令行参数的有效个数,是信号量集的个数+2
	if (argc != nsems + 2)
		err_quit("%d semaphores in set, %d values specified", nsems, argc-2);

		/* 4allocate memory to hold all the values in the set, and store */
	ptr = Calloc(nsems, sizeof(unsigned short)); //分配内存空间"信号量集的个数*unsigned short"
	arg.array = ptr; //使ptr指向这块内存
	for (i = 0; i < nsems; i++) //用循环,把命令行参数第三个参数开始的值,放入ptr指向的数组中
		ptr[i] = atoi(argv[i + 2]); //因为已经保证的命令行参数的个数与信号量集的个数是一致的,所以此地循环不会内存溢出,很安全
	Semctl(semid, 0, SETALL, arg); //把值设置进信号量

	exit(0);
}
