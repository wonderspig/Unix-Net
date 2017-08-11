#include	"unpipc.h"

struct mq_attr	attr;	/* mq_maxmsg and mq_msgsize both init to 0 */

int
main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;

	flags = O_RDWR | O_CREAT; //设置标志为读写,创建
	//格式化命令行参数为-e,-m,-z, 其中m,z带后缀属性(指针optarg为当前选项后跟的属性内容)
	while ( (c = Getopt(argc, argv, "em:z:")) != -1) {
		switch (c) {
		case 'e': //有-e参数就加上排他标志
			flags |= O_EXCL;
			break;

		case 'm': //-m参数就把m后跟的属性填写入attr.mq_maxmsg这个成员
			attr.mq_maxmsg = atol(optarg);
			break;

		case 'z': //-z参数就是z后跟的属性填写入attr.mq_msgsize这个成员
			attr.mq_msgsize = atol(optarg);
			break;
		}
	}
	if (optind != argc - 1) //optind为下一个选项的索引
		err_quit("usage: mqcreate [ -e ] [ -m maxmsg -z msgsize ] <name>");

	if ((attr.mq_maxmsg != 0 && attr.mq_msgsize == 0) ||
		(attr.mq_maxmsg == 0 && attr.mq_msgsize != 0)) //必须两个选项都设置才是有效的，所以只有一个选项有值,则报错
		err_quit("must specify both -m maxmsg and -z msgsize");

	mqd = Mq_open(argv[optind], flags, FILE_MODE,
				  (attr.mq_maxmsg != 0) ? &attr : NULL); //根据设置好的选项,标志,用户读写权限创建消息队列

	Mq_close(mqd);
	exit(0);
}
