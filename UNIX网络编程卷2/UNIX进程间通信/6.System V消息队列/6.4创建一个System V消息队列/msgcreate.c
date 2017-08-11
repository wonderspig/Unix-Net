#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		c, oflag, mqid;

	oflag = SVMSG_MODE | IPC_CREAT; //创建消息队列的权限位以及不管是否存在都创建消息队列,保存在oflag中
	//如果有-e参数,那么就加上"已存在就报错"位
	while ( (c = Getopt(argc, argv, "e")) != -1) {
		switch (c) { //从命令行提取出来的参数
		case 'e': //如果命令行参数为-e
			oflag |= IPC_EXCL; //就让oflag加上IPC_EXCL
			break;
		}
	}
	if (optind != argc - 1) //确保-e不是最后一个参数(-e后面的参数是消息队列的路径,这个路径用来生成IPC的KEY)
		err_quit("usage: msgcreate [ -e ] <pathname>");

	//创建消息队列
	//用Ftok来使命令行参数穿进来的路径转换为KEY
	//用这个KEY和保存在oflag中的权限位来创建消息队列
	mqid = Msgget(Ftok(argv[optind], 0), oflag);
	exit(0);
}
