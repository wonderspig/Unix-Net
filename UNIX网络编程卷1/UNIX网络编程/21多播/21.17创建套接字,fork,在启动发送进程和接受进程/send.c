#include	"unp.h"
#include	<sys/utsname.h>

#define	SENDRATE	5		/* send one datagram every five seconds */

void
send_all(int sendfd, SA *sadest, socklen_t salen)
{
	char		line[MAXLINE];		/* hostname and process ID */
	struct utsname	myname;
	//uname函数获得主机名并构造一个包含主机名和进程ID的输出行
	if (uname(&myname) < 0)
		err_sys("uname error");;
	//打印进程ID和主机名
	snprintf(line, sizeof(line), "%s, %d\n", myname.nodename, getpid());

	for ( ; ; ) {
		//发送一个UDP数据包
		Sendto(sendfd, line, strlen(line), 0, sadest, salen);

		sleep(SENDRATE); //睡5秒
	}
}
