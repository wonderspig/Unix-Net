#include	"unpipc.h"

#define	MAX_DATA	64*1024
#define	MAX_NMESG	4096
#define	MAX_NIDS	4096
int		max_mesg;

struct mymesg {
  long	type;
  char	data[MAX_DATA];
} mesg;

int
main(int argc, char **argv)
{
	int		i, j, msqid, qid[MAX_NIDS];

		/* 4first try and determine maximum amount of data we can send */
	msqid = Msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT); //创建私有消息队列
	mesg.type = 1; //消息类型置为1
	for (i = MAX_DATA; i > 0; i -= 128) { //由大到小发送数据,测试出系统的msgmax(每个消息的最大字节数)
		if (msgsnd(msqid, &mesg, i, 0) == 0) { //由大到小发送数据
			printf("maximum amount of data per message = %d\n", i); //成功后打印的信息
			max_mesg = i; //记录当前发送数据的大小(也就是限制值)
			break;
		}
		if (errno != EINVAL) //排除msgsnd函数参数无效的情况
			err_sys("msgsnd error for length %d", i);
	}
	if (i == 0) //直到发送数据的大小减到0,仍然无法发送
		err_quit("i == 0");
	Msgctl(msqid, IPC_RMID, NULL); //删除这个消息队列

		/* 4see how many messages of varying size can be put onto a queue */
	mesg.type = 1; //消息类型置为1
	//在单条消息的最大字节数限制值之中,单条消息字节数从小到大依次发送,查找能够一个消息队列上最多可以放入多少消息
	//我们期待较小的消息可以测试出每个消息队列中最多可以存放多少条消息
	//我们期待较大的消息可以测试出每个消息队列中可以容纳的最大字节数
	//i是每条消息的字节数,j是一共发了几条消息
	for (i = 8; i <= max_mesg; i *= 2) {
		msqid = Msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT); //创建私有消息队列
		for (j = 0; j < MAX_NMESG; j++) { //j是用来计算,发送了多少条消息
			if (msgsnd(msqid, &mesg, i, IPC_NOWAIT) != 0) { //把消息放入消息队列
				if (errno == EAGAIN) //错误代码为再一次尝试
					break;
				err_sys("msgsnd error, i = %d, j = %d", i, j); //发送失败,打印i和j的值
				break;
			}
		}
		//发送成功
		printf("%d %d-byte messages were placed onto queue,", j, i); //打印循环结束后i和j的值
		printf(" %d bytes total\n", i*j); //此时消息队列中总共容纳的字节数
		Msgctl(msqid, IPC_RMID, NULL); //删除这个消息队列
	}

		/* 4see how many identifiers we can "open" */
	//通过不停的打开消息队列,来检测系统中消息队列最多可以打开多少个
	mesg.type = 1; //消息类型设置为1
	for (i = 0; i <= MAX_NIDS; i++) { //用循环不停的打开消息队列
		if ( (qid[i] = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT)) == -1) { //不停的打开消息队列直到失败
			printf("%d identifiers open at once\n", i); //i就代表可以打开的消息队列的最大个数
			break;
		}
	}
	for (j = 0; j < i; j++) //用循环删除刚才创建的所有消息队列
		Msgctl(qid[j], IPC_RMID, NULL); //删除消息队列

	exit(0);
}
