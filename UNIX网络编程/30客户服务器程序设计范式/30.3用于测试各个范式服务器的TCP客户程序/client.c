#include	"unp.h"

#define	MAXN	16384		/* max # bytes to request from server */

int
main(int argc, char **argv)
{
	int		i, j, fd, nchildren, nloops, nbytes;
	pid_t	pid;
	ssize_t	n;
	char	request[MAXLINE], reply[MAXN];

	if (argc != 6) //命令行参数不等于5,则出错退出
		err_quit("usage: client <hostname or IPaddr> <port> <#children> "
				 "<#loops/child> <#bytes/request>");

	nchildren = atoi(argv[3]); //由客户fork的子进程数
	nloops = atoi(argv[4]); //每个子进程发送给服务器的请求连接数
	nbytes = atoi(argv[5]); //每个请求要求服务器返送的数据字节数
	//"每个请求要求服务器返送的数据字节数"打印成表达式格式
	snprintf(request, sizeof(request), "%d\n", nbytes); /* newline at end */

	for (i = 0; i < nchildren; i++) { //循环创建传参数目的子进程
		if ( (pid = Fork()) == 0) {		/* child 在紫禁城中*/
			for (j = 0; j < nloops; j++) {
				fd = Tcp_connect(argv[1], argv[2]); //根据服务器的主机名和端口号发起连接

				Write(fd, request, strlen(request)); //把请求发送给服务器

				if ( (n = Readn(fd, reply, nbytes)) != nbytes) //读取服务器应答(服务器按照请求返回的字节数)
					err_quit("server returned %d bytes", n);

				Close(fd);		/* TIME_WAIT on client, not server 关闭连接到服务器的套接字*/
			}
			printf("child %d done\n", i); //打印创建了几个子进程
			exit(0); //子进程退出
		}
		/* parent loops around to fork() again */
	}
	//父进程中
	while (wait(NULL) > 0)	/* now parent waits for all children 等待所有的子进程结束并回首资源 */
		;
	if (errno != ECHILD) //错误为没有子进程需要等待,这个就是预期中的错误,别的错误不可接受
		err_sys("wait error");

	exit(0); //父进程退出
}
