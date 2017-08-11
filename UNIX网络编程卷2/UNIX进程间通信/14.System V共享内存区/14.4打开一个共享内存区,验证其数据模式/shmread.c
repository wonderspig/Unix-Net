#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		i, id;
	struct shmid_ds	buff;
	unsigned char	c, *ptr;

	if (argc != 2) //如果命令行参数不等于2,则出错
		err_quit("usage: shmread <pathname>");
	//Shmget打开一个已存在的共享内存区,命令行第一个参数所代表的路径生成key,权限位是SVSHM_MODE
	id = Shmget(Ftok(argv[1], 0), 0, SVSHM_MODE);
	ptr = Shmat(id, NULL, 0); //映射id所代表的共享内存区到本进程
	Shmctl(id, IPC_STAT, &buff); //获得这个共享内存区的状态,填写buff结构体,为了获得共享内存区的大小

		/* 4check that ptr[0] = 0, ptr[1] = 1, etc. */
	for (i = 0; i < buff.shm_segsz; i++) //用共享内存区的大小进行循环,读取共享内存区
		if ( (c = *ptr++) != (i % 256)) //用写入相同的方式进行读取
			err_ret("ptr[%d] = %d", i, c);

	exit(0);
}
