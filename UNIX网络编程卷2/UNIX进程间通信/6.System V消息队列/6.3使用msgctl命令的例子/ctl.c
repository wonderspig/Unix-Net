#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int				msqid; //消息队列标识符
	struct msqid_ds	info;  //管理消息队列的结构体
	struct msgbuf	buf;   //发送消息队列的缓冲区<sys/msg.h>

	msqid = Msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT); //创建一个消息队列,不管存不存在都创建一个新的消息队列

	buf.mtype = 1; //消息类型
	buf.mtext[0] = 1; //消息数据
	Msgsnd(msqid, &buf, 1, 0); //把消息放入消息队列

	Msgctl(msqid, IPC_STAT, &info); //把消息队列的数据放入info结构体中
	printf("read-write: %03o, cbytes = %lu, qnum = %lu, qbytes = %lu\n",
		   info.msg_perm.mode & 0777, (ulong_t) info.msg_cbytes,
		   (ulong_t) info.msg_qnum, (ulong_t) info.msg_qbytes); //打印这个结构体的信息

	system("ipcs -q"); //运行终端命令

	Msgctl(msqid, IPC_RMID, NULL); //删除这个消息队列
	exit(0);
}
