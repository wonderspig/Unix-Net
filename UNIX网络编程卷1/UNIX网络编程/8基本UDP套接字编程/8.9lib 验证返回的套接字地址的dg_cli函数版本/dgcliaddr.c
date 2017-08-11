#include	"unp.h"

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int				n;
	char			sendline[MAXLINE], recvline[MAXLINE + 1];
	socklen_t		len;
	struct sockaddr	*preply_addr;

	preply_addr = Malloc(servlen); //为地址结构动态分配空间

	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入(fp)读入一行,存放在sendline中

		//发送recvline中的内容至服务器(sockfd),发送地址为pservaddr,地址结构体长度为servlen
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		len = servlen; //把服务器地址长度复制一份(用来调用Recvfrom函数)

		//从服务器(sockfd)中接受内容内容,并把接受端的地址存放入preply_addr中
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
		//如果接受端的地址长度和发送时的地址长度不相等
		//或者发送时的地址和接受时的地址不一致
		if (len != servlen || memcmp(pservaddr, preply_addr, len) != 0) {
			printf("reply from %s (ignored)\n",
					Sock_ntop(preply_addr, len)); //协议无关的把二进制格式地址preply_addr,打印成字符串
			continue;
		}

		recvline[n] = 0;	/* null terminate 给接受缓冲区末尾加入'\0'字节 */
		Fputs(recvline, stdout); //把接受缓冲区中的数据打印到标准输出
	}
}
