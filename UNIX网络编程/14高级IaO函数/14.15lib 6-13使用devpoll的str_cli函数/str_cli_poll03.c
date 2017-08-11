#include	"unp.h"
#include	<sys/devpoll.h>

void
str_cli(FILE *fp, int sockfd)
{
	int		stdineof;
	char		buf[MAXLINE];
	int		n;
	int		wfd;
	struct pollfd	pollfd[2];
	struct dvpoll	dopoll;
	int		i;
	int		result;

	wfd = Open("/dev/poll", O_RDWR, 0); //打开"/dev/poll"

	//定义pollfd结构体数组
	pollfd[0].fd = fileno(fp);
	pollfd[0].events = POLLIN;
	pollfd[0].revents = 0;

	pollfd[1].fd = sockfd;
	pollfd[1].events = POLLIN;
	pollfd[1].revents = 0;

	Write(wfd, pollfd, sizeof(struct pollfd) * 2); //把pollfd结构体数组写入"/dev/poll"中

	stdineof = 0; //设定标志量
	for ( ; ; ) {
		/* block until /dev/poll says something is ready 
		   阻塞直到/dev/poll说有哪些时间准备好了*/

		//设置dopoll参数,调用Ioctl时,此参数用来通知调"/dev/poll"需要注意哪些事情
		dopoll.dp_timeout = -1; //永久阻塞
		dopoll.dp_nfds = 2; //缓冲区中的数组的元素个数
		dopoll.dp_fds = pollfd; //缓冲区为pollfd数组
		result = Ioctl(wfd, DP_POLL, &dopoll); //都设置好了，调用Ioctl

		/* loop through ready file descriptors */
		for (i = 0; i < result; i++) { //循环遍历每一个结果
			if (dopoll.dp_fds[i].fd == sockfd) { //如果准备好的是sockfd
				/* socket is readable */
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) { //那就从sockfd中读
					if (stdineof == 1) //检查标志量,1代表收到eof
						return;		/* normal termination */
					else
						err_quit("str_cli: server terminated prematurely");
				}

				Write(fileno(stdout), buf, n); //把从服务器sockfd中读来的数据写到标准输出
			} else { //否则就是标准输入fp准备好了
				/* input is readable */
				if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) { //从标准输入fp中读入一行,存放在buf中
					stdineof = 1; //收到eof设置标志量
					Shutdown(sockfd, SHUT_WR);	/* send FIN 向服务器发送fin,写端关闭会让连接在读完所有数据后关闭连接 */
					continue; //继续循环
				}

				Writen(sockfd, buf, n); //否则把从标准输入读到的数据buf,发送到服务器sockfd
			}
		}
	}
}
