#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		semid, nsems, i;
	struct semid_ds	seminfo;
	unsigned short	*ptr;
	union semun	arg;

	if (argc != 2) //如果命令行参数不等于2,则报错
		err_quit("usage: semgetvalues <pathname>");

		/* 4first get the number of semaphores in the set */
	semid = Semget(Ftok(argv[1], 0), 0, 0); //用命令行的第一个参数创建key,来打开已存在的信号量
	arg.buf = &seminfo;//arg.buf为semid_ds结构体,获得当前信号量集的当前状态
	Semctl(semid, 0, IPC_STAT, arg); //用Semctl函数获得当前信号量状态,填充semid_ds结构体
	nsems = arg.buf->sem_nsems; //获得信号量集当前状态中的"信号量数组的个数"信息

		/* 4allocate memory to hold all the values in the set */
	ptr = Calloc(nsems, sizeof(unsigned short)); //分配内存空间"信号量集的个数*unsigned short"
	arg.array = ptr; //使ptr指向这块内存

		/* 4fetch the values and print */
	Semctl(semid, 0, GETALL, arg); //获得当前信号量集中所有信号量的值
	for (i = 0; i < nsems; i++) //用循环打印出信号量集中所有信号量的值
		printf("semval[%d] = %d\n", i, ptr[i]);

	exit(0);
}
