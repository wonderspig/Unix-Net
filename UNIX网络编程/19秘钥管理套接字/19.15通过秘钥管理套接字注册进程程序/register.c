#include "unp.h"
#include <net/pfkeyv2.h>

void
print_sadb_msg(struct sadb_msg *msg, int msglen);
int
getsatypebyname(char *name);

/* include sadb_register */
void
sadb_register(int type)
{
	int s; //秘钥管理套接字
	char buf[4096];	/* XXX 缓冲区(用于接受内核返回消息的缓冲区)*/
	struct sadb_msg msg; //秘钥管理消息首部
	//int goteof;
	int mypid; //本进程pid

	s = Socket(PF_KEY, SOCK_RAW, PF_KEY_V2); //创建秘钥管理套接字,PF_KEY_V2版本

	mypid = getpid(); //保存一下本进程的pid

	/* Build and write SADB_REGISTER request */
	bzero(&msg, sizeof(msg)); //把秘钥管理套接字消息首部置空
	msg.sadb_msg_version = PF_KEY_V2; //秘钥管理套接字的消息版本PF_KEY_V2
	msg.sadb_msg_type = SADB_REGISTER; //秘钥管理套接字消息类型SADB_REGISTER,注册成本程序为SADB_ACQUIRE消息的应答者
	msg.sadb_msg_satype = type; //SA类型: SADB_SATYPE_AH或者SADB_SATYPE_ESP
	//秘钥管理消息的长度:因为注册本进程的MSG类型(SADB_REGISTER),不需要任何扩展首部,所以,消息长度就是MSG的首部长度
	msg.sadb_msg_len = sizeof(msg) / 8;
	msg.sadb_msg_pid = mypid; //本进程pid
	printf("Sending register message:\n");
	print_sadb_msg(&msg, sizeof(msg)); //打印本msg消息(秘钥管理套接字消息)
	Write(s, &msg, sizeof(msg)); //把这个秘钥管理套接字消息写入内核

	printf("\nReply returned:\n");
	/* Read and print SADB_REGISTER reply, discarding any others */
	for (;;) {
		int msglen; //从内核接受应答消息的长度
		struct sadb_msg *msgp; //msg结构体(秘钥管理套接字消息首部)

		msglen = Read(s, &buf, sizeof(buf)); //用read函数从秘钥管理套接字读取内核应答,存入buf缓冲区
		msgp = (struct sadb_msg *)&buf; //把buf缓冲区转换成msg结构体,从而读取其中内容
		if (msgp->sadb_msg_pid == mypid && //如果消息中记录的pid是本进程的pid
			msgp->sadb_msg_type == SADB_REGISTER) { //且消息类型是SADB_REGISTER(注册成本程序为SADB_ACQUIRE消息的应答者)
			print_sadb_msg(msgp, msglen); //打印这条秘钥管理套接字消息
			break;
		}
	}
	close(s);
}
/* end sadb_register */

int
main(int argc, char **argv)
{
	int satype = SADB_SATYPE_UNSPEC;
	int c;

	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "t:")) != -1) {
		switch (c) {
		case 't':
			if ((satype = getsatypebyname(optarg)) == -1)
				err_quit("invalid -t option %s", optarg);
			break;

		default:
			err_quit("unrecognized option: %c", c);
		}
	}

	if (satype == SADB_SATYPE_UNSPEC) {
		err_quit("must specify SA type");
	}

	sadb_register(satype);
}
