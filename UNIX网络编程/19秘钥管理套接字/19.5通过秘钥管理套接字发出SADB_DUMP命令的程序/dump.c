#include "unp.h"
#include <net/pfkeyv2.h>

void
print_sadb_msg(struct sadb_msg *msg, int msglen);
int
getsatypebyname(char *name);

/* include sadb_dump */
void
sadb_dump(int type)
{
	int s;
	char buf[4096];
	struct sadb_msg msg;
	int goteof;

	//创建PF_KEY套接字,写出的所有消息必须把消息版本设置为PF_KEY_V2
	s = Socket(PF_KEY, SOCK_RAW, PF_KEY_V2);

	/* Build and write SADB_DUMP request */
	bzero(&msg, sizeof(msg)); //清零结构体sadb_msg
	msg.sadb_msg_version = PF_KEY_V2; //设置消息版本
	msg.sadb_msg_type = SADB_DUMP; //消息类型(请求):倾泻出SADB
	msg.sadb_msg_satype = type; //消息类型(接受):传参要求
	msg.sadb_msg_len = sizeof(msg) / 8; //消息长度
	msg.sadb_msg_pid = getpid(); //本进程pid
	printf("Sending dump message:\n");
	print_sadb_msg(&msg, sizeof(msg));
	Write(s, &msg, sizeof(msg)); //把填写好的msg函数写入到套接字

	printf("\nMessages returned:\n");
	/* Read and print SADB_DUMP replies until done */
	goteof = 0; //设置标志.消息还没有结束
	while (goteof == 0) {
		int msglen;
		struct sadb_msg *msgp;

		msglen = Read(s, &buf, sizeof(buf)); //从套接字中读取内核的回应
		msgp = (struct sadb_msg *)&buf; //把buf强转成sadb_msg类型的指针
		print_sadb_msg(msgp, msglen);
		if (msgp->sadb_msg_seq == 0) //末尾的消息以sadb_msg_seq成员等于0,来表示
			goteof = 1;
	}
	close(s); //关闭套接字
}

int
main(int argc, char **argv)
{
	int satype = SADB_SATYPE_UNSPEC;
	int c;

	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "t:")) != -1) { //命令行参数指定消息类型
		switch (c) {
		case 't':
			if ((satype = getsatypebyname(optarg)) == -1) //把消息类型填入变量satype
				err_quit("invalid -t option %s", optarg);
			break;

		default:
			err_quit("unrecognized option: %c", c);
		}
	}

	sadb_dump(satype); //根据需要的消息类型打印消息的具体信息
}
/* end sadb_dump */
