#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, id, oflag;
	char	*ptr;
	size_t	length;

	oflag = SVSHM_MODE | IPC_CREAT; //权限位设置,没有就创建,有就打开
	while ( (c = Getopt(argc, argv, "e")) != -1) { //从命令行读取-e参数
		switch (c) { //筛选参数
		case 'e': //如果是-e选项
			oflag |= IPC_EXCL; //则或上IPC_EXCL位
			break;
		}
	}
	if (optind != argc - 2) //下一个命令行参数,必须是命令行参数的倒数第二个,否则报错
		err_quit("usage: shmget [ -e ] <pathname> <length>");
	length = atoi(argv[optind + 1]); //命令行的最后一个参数转换成数值形式,存放在length中
	//调用Shmget创建共享内存,使用命令行倒数第二个参数所代表的路径生成key,内存大小为length,权限位为oflag
	id = Shmget(Ftok(argv[optind], 0), length, oflag);
	ptr = Shmat(id, NULL, 0); //映射内存到本进程,内存IPC名字是id,不指定地址,权限位默认

	exit(0);
}
