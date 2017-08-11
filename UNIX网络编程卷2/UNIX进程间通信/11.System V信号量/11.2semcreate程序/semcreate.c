#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, oflag, semid, nsems;

	oflag = 0644 | IPC_CREAT; //标志位设置成SVSEM_MODE加创建IPC
	while ( (c = Getopt(argc, argv, "e")) != -1) { //从命令行获得参数
		switch (c) { //筛选参数
		case 'e': //如果参数位-e
			oflag |= IPC_EXCL; //那么就加上排他参数
			break;
		}
	}
	if (optind != argc - 2) //下一个命令行参数不为倒数第二个参数
		err_quit("usage: semcreate [ -e ] <pathname> <nsems>");
	nsems = atoi(argv[optind + 1]); //获取命令行最后一个参数(最后一个参数是"信号量集"的数目)
	//用命令行倒数第二个参数所制定的路径生成key,来创建信号量,"信号量集"的个数为nsems,权限位是oflag
	semid = Semget(Ftok(argv[optind], 0), nsems, oflag);
	exit(0);
}
