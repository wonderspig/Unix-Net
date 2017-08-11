#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, i, flag, semid, nops;
	struct sembuf	*ptr;

	flag = 0; //Semop的第三个参数
	while ( (c = Getopt(argc, argv, "nu")) != -1) { //获得命令行参数:-n,-u
		switch (c) { //筛选命令行参数
		case 'n': //-n参数附加上IPC_NOWAIT位
			flag |= IPC_NOWAIT;		/* for each operation */
			break;

		case 'u': //-u参数附加上SEM_UNDO位
			flag |= SEM_UNDO;		/* for each operation */
			break;
		}
	}
	//命令行参数中,当前指向的参数往后数,至少还有两个以上的参数(分别为路径名,和选项)
	if (argc - optind < 2)			/* argc - optind = #args remaining */
		err_quit("usage: semops [ -n ] [ -u ] <pathname> operation ...");

	semid = Semget(Ftok(argv[optind], 0), 0, 0); //用命令行的参数创建key,来打开已存在的信号量
	optind++; //下一个命令行参数跳过路径名,指向选项
	nops = argc - optind; //argc是命令行参数的总数,npos是选项的个数

		/* 4allocate memory to hold operations, store, and perform */
	ptr = Calloc(nops, sizeof(struct sembuf)); //分配内存空间"选项的个数*unsigned short"
	for (i = 0; i < nops; i++) { //用选项的个数进行循环(每个选项收拾包含三个成员的结构体,分别在循环中动态为他们赋值)
		ptr[i].sem_num = i; //信号量集中第几个信号量
		ptr[i].sem_op = atoi(argv[optind + i]);	/* <0, 0, or >0 信号量的设置值*/
		ptr[i].sem_flg = flag; //信号量的标志(尝试操作,还是真的直接写入)
	}
	Semop(semid, ptr, nops); //操作信号量集中的所有信号量

	exit(0);
}
