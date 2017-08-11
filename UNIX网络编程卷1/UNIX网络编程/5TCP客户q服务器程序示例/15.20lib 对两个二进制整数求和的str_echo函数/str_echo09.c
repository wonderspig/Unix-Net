#include	"unp.h"
#include	"sum.h"

void
str_echo(int sockfd)
{
	ssize_t			n;
	struct args		args;
	struct result	result;

	for ( ; ; ) {
		if ( (n = Readn(sockfd, &args, sizeof(args))) == 0) //从连接客户端的套接字中读取args这个结构体
			return;		/* connection closed by other end 读到EOF就返回*/

		result.sum = args.arg1 + args.arg2; //结构体中的两个成员相加并存放入result结构体
		Writen(sockfd, &result, sizeof(result)); //把result结构体写入连接客户端的套接字
	}
}
