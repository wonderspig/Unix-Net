#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		i, id;
	struct shmid_ds	buff;
	unsigned char	*ptr;

	if (argc != 2) //命令行参数不等于1,则报错
		err_quit("usage: shmwrite <pathname>");
	//Shmget函数打开一个已存在的共享内存区,使用命令行的第一个参数创建key,权限位是SVSHM_MODE
	id = Shmget(Ftok(argv[1], 0), 0, SVSHM_MODE);
	ptr = Shmat(id, NULL, 0); //映射id所代表的共享内存区
	Shmctl(id, IPC_STAT, &buff); //获得这个共享内存区的状态,填写buff结构体,为了获得共享内存区的大小

		/* 4set: ptr[0] = 0, ptr[1] = 1, etc. */
	for (i = 0; i < buff.shm_segsz; i++) //用循环往共享内存区中写入数据
		*ptr++ = i % 256; //填入一个256之内的数字

	exit(0);
}
