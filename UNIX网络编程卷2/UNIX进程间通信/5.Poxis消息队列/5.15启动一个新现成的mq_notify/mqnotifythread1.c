#include	"unpipc.h"

mqd_t	mqd;
struct mq_attr	attr;
struct sigevent	sigev;

static void	notify_thread(union sigval);		/* our thread function */

int
main(int argc, char **argv)
{
	if (argc != 2) //如果命令行参数不为2,则报错
		err_quit("usage: mqnotifythread1 <name>");

	mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK); //以只读非阻塞的方式打开命令行指定的消息队列
	Mq_getattr(mqd, &attr); //获得这个消息队列的属性

	sigev.sigev_notify = SIGEV_THREAD; //通知方式为新开一个线程等待来处理消息队列
	sigev.sigev_value.sival_ptr = NULL; //传递给线程函数的参数为null
	sigev.sigev_notify_function = notify_thread; //使用的线程函数
	sigev.sigev_notify_attributes = NULL; //新线程的属性
	Mq_notify(mqd, &sigev); //把当前线程注册为接收该消息队列的通知

	for ( ; ; )
		pause();		/* each new thread does everything 休眠等待通知*/

	exit(0);
}
//线程函数
static void
notify_thread(union sigval arg)
{
	ssize_t	n;
	void	*buff;

	printf("notify_thread started\n");
	buff = Malloc(attr.mq_msgsize); //用队列属性中的单个消息最大长度来动态分配缓冲区
	Mq_notify(mqd, &sigev);			/* reregister 重新注册当前进程为接收消息队列通知的进程 */

	while ( (n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) { //循环从队列中取出消息,直到出错为止
		printf("read %ld bytes\n", (long) n); //打印取出消息的大小
	}
	if (errno != EAGAIN) //消息队列中没有消息了(其他错误报错退出)
		err_sys("mq_receive error");

	free(buff);
	pthread_exit(NULL);
}
