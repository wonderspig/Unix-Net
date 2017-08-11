/* include udpserv1 */
#include	"unpifi.h"

void	mydg_echo(int, SA *, socklen_t, SA *);

int
main(int argc, char **argv)
{
	int					sockfd;
	const int			on = 1;
	pid_t				pid;
	struct ifi_info		*ifi, *ifihead;
	struct sockaddr_in	*sa, cliaddr, wildaddr;
	//获得所有IP4接口的信息,且关心别名
	//用循环遍历链表
	for (ifihead = ifi = Get_ifi_info(AF_INET, 1); 
		 ifi != NULL; ifi = ifi->ifi_next) {

			/*4bind unicast address */
		sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建IP4套接字,UDP
		//打开套接字选项SO_REUSEADDR,因为要给所有IP地址捆绑同一个端口
		Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		sa = (struct sockaddr_in *) ifi->ifi_addr; //提取链表节点中的地址,填入sa(单播地址)
		sa->sin_family = AF_INET; //地址族设置为IP4
		sa->sin_port = htons(SERV_PORT); //填入端口号
		Bind(sockfd, (SA *) sa, sizeof(*sa)); //绑定这个天蝎好的地址sa
		//Sock_ntop函数转换二进制地址到表达式格式
		printf("bound %s\n", Sock_ntop((SA *) sa, sizeof(*sa))); //打印绑定的地址信息
		//创建子进程
		if ( (pid = Fork()) == 0) {		/* child 在子进程中 */
			//mydg_echo函数:等待任意数据报到达套接字,并回射给发送者
			mydg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr), (SA *) sa);
			exit(0);		/* never executed */
		}
/* end udpserv1 */
/* include udpserv2 */
		//在父进程中处理目的地址为广播地址的UDP数据报
		if (ifi->ifi_flags & IFF_BROADCAST) { //如果接口支持广播
				/* 4try to bind broadcast address */
			sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建套接字IP4地址族,UDP
			//打开套接字选项SO_REUSEADDR,因为要给所有IP地址捆绑同一个端口
			Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

			sa = (struct sockaddr_in *) ifi->ifi_brdaddr; //把广播地址填写入sa中 
			sa->sin_family = AF_INET; //地址族为IP4
			sa->sin_port = htons(SERV_PORT); //填写入端口号
			if (bind(sockfd, (SA *) sa, sizeof(*sa)) < 0) { //绑定这个广播地址
				//EADDRINUSE错误,重复绑定
				//如果这个接口有别名(多个地址处于同一个子网),那么这些单播地址会对应同一个广播地址
				//所以只有第一次绑定会成功
				if (errno == EADDRINUSE) { 
					printf("EADDRINUSE: %s\n",
						   Sock_ntop((SA *) sa, sizeof(*sa)));
					Close(sockfd);
					continue;
				} else //其他错误
					err_sys("bind error for %s",
							Sock_ntop((SA *) sa, sizeof(*sa)));
			}
			printf("bound %s\n", Sock_ntop((SA *) sa, sizeof(*sa))); //打印已经绑定的信息
			//创建一个子进程
			if ( (pid = Fork()) == 0) {		/* child 在子进程中*/
				//mydg_echo函数:等待任意数据报到达套接字,并回射给发送者
				mydg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr),
						  (SA *) sa);
				exit(0);		/* never executed */
			}
		}
	}
/* end udpserv2 */
/* include udpserv3 */
		/* 4bind wildcard address */

	//父进程中再处理单播地址和广播地址之外的其他地址
	//能够到达这个套接字的应该只有受限的广播地址(255.255.255.255)

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建套接字IP4地址族,UDP
	//打开套接字选项SO_REUSEADDR,因为要给所有IP地址捆绑同一个端口
	Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&wildaddr, sizeof(wildaddr)); //初始化清空地址结构体wildaddr
	wildaddr.sin_family = AF_INET; //IP4地址族
	wildaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配地址
	wildaddr.sin_port = htons(SERV_PORT); //填写端口号
	Bind(sockfd, (SA *) &wildaddr, sizeof(wildaddr)); //绑定填写好的这个地址wildaddr
	printf("bound %s\n", Sock_ntop((SA *) &wildaddr, sizeof(wildaddr))); //打印已绑定的俄地址信息
	//创建子进程
	if ( (pid = Fork()) == 0) {		/* child 再子进程中 */
		//mydg_echo函数:等待任意数据报到达套接字,并回射给发送者
		mydg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr), (SA *) sa);
		exit(0);		/* never executed */
	}
	exit(0);
}
/* end udpserv3 */

/* include mydg_echo */
//mydg_echo函数:等待任意数据报到达套接字,并回射给发送者
void
mydg_echo(int sockfd, SA *pcliaddr, socklen_t clilen, SA *myaddr)
{
	int			n;
	char		mesg[MAXLINE];
	socklen_t	len;

	for ( ; ; ) {
		len = clilen;
		//从绑定地址的套接字sockfd上接收数据报,存放在mesg中
		//对端地址填写在pcliaddr中
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len); 
		//打印相关信息
		printf("child %d, datagram from %s", getpid(),
			   Sock_ntop(pcliaddr, len));
		printf(", to %s\n", Sock_ntop(myaddr, clilen));

		Sendto(sockfd, mesg, n, 0, pcliaddr, len); //把相同的消息发送给发送者
	}
}
/* end mydg_echo */
