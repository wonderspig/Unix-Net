#include	"unp.h"
#include	"sum.h"

void
str_cli(FILE *fp, int sockfd)
{
	char			sendline[MAXLINE];
	struct args		args;
	struct result	result;

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从fp(标准输入)读入一行

		////把读到的这一行转换成arg1,arg2,填写入结构
		if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) { 
			printf("invalid input: %s", sendline); //打印错误信息
			continue;
		}
		Writen(sockfd, &args, sizeof(args)); //把填写好的结构体写入连接服务器的套接字中

		if (Readn(sockfd, &result, sizeof(result)) == 0) //从连接服务器的套接字中读取结果结构体
			err_quit("str_cli: server terminated prematurely");

		printf("%ld\n", result.sum); //打印结果结构体中的内容
	}
}
