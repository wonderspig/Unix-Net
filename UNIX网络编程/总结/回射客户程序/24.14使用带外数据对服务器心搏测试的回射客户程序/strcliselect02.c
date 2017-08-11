#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	int		n;

	//标记,标准输入没有收到EOF时标记为0,收到EOF时标记为1
	stdineof = 0; //初始化标记
	FD_ZERO(&rset); //初始化清空描述符集
	heartbeat_cli(sockfd, 1, 5); //设置客户的心搏特性
	for ( ; ; ) {
		if (stdineof == 0) //如果没有收到EOF(如果从标准输入读到了EOF,select就不再关心标准输入的描述符)
			FD_SET(fileno(fp), &rset); //把文件指针(标准输入),设置进描述符集
		FD_SET(sockfd, &rset); //把连接到服务器的套接字设置进描述符集
		maxfdp1 = max(fileno(fp), sockfd) + 1; //取两个描述符集的最大值并且+1
		if( (n = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0){ //用以上参数调用select,同时关心这两个描述符的可读状态
			if (errno == EINTR) //在heartbeat_cli这个函数中,捕捉了SIGALARM和SIGURG信号,所以select必须处理被中断的系统调用
				continue;
			else
				err_sys("select error");
		}

		//当select返回时判断是那个描述符准备好了
		if (FD_ISSET(sockfd, &rset)) {	/* socket is readable 如果是套接字描述符准备好了*/
			if ( (n = Read(sockfd, buf, MAXLINE)) == 0) { //从套接字描述符中读一行
				//进入此分支代表从套接字上读到了EOF(服务器进程奔溃)
				if (stdineof == 1) //再从套接字上读到EOF的同时,从标准输入也读到EOF
					return;		/* normal termination 函数返回,将会退出进程*/
				else
					err_quit("str_cli: server terminated prematurely"); //打印错误消息
			}

			Writen(fileno(stdout), buf, n); //把从套接字中读到的数据写入标准输出
		}

		if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable 如果是fp(标准输入准备好了)*/
			if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) { //从标准输入读入一行
				stdineof = 1; //标准输入读入EOF,所以把标志置为1
				/* send FIN 向服务器发送FIN,关闭套接字连接的写这一半 */
				Shutdown(sockfd, SHUT_WR);	
				FD_CLR(fileno(fp), &rset); //删除描述符集中标准输入的描述符
				continue; //继续循环
			}

			Writen(sockfd, buf, n); //把从标准输入读到的内容写入与服务器连接的套接字中
		}
	}
}
