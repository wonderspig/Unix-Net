#include	"unpipc.h"

mqd_t	mqd;
void	*buff;
struct mq_attr	attr;
struct sigevent	sigev;

static void	sig_usr1(int);

int
main(int argc, char **argv)
{
	if (argc != 2) //命令行参数不等于1,则报错
		err_quit("usage: mqnotifysig1 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY); //打开命令行参数所指的消息队列
	Mq_getattr(mqd, &attr); //获得这个消息队列的属性
	buff = Malloc(attr.mq_msgsize); //通过消息队列属性中每条消息的最大大小这个成员，动态分配缓存空间

		/* 4establish signal handler, enable notification */
	Signal(SIGUSR1, sig_usr1); //注册SIGUSR1的信号处理程序
	sigev.sigev_notify = SIGEV_SIGNAL; //用信号的方式通知当前进程
	sigev.sigev_signo = SIGUSR1; //如果是用信号通知,则著名用哪个信号通知
	Mq_notify(mqd, &sigev); //调用Mq_notify把当前进程注册为接收该队列的通知

	for ( ; ; )
		pause();		/* signal handler does everything 睡眠以等待信号的到来 */
	exit(0);
}
//信号处理程序
static void
sig_usr1(int signo)
{
	ssize_t	n;

	Mq_notify(mqd, &sigev);			/* reregister first 在把消息队列中的消息读取出来之前，重新注册调用进程接收消息队列的通知 */
	n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL); //注册完成后从消息队列中取消息
	printf("SIGUSR1 received, read %ld bytes\n", (long) n); //打印这个消息的大小
	return;
}
