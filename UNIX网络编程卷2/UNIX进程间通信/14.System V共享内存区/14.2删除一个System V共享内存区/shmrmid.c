#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		id;

	if (argc != 2) //如果命令行参数不等于2,则报错
		err_quit("usage: shmrmid <pathname>");
    //调用Shmget打开已存在的共享内存,使用命令行倒数第一个参数所代表的路径生成key,权限位为SVSHM_MODE
	id = Shmget(Ftok(argv[1], 0), 0, SVSHM_MODE);
	Shmctl(id, IPC_RMID, NULL); //使用IPC_RMID命令,删除共享内存

	exit(0);
}
