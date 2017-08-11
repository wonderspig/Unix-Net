/* include dgcli011 */
#include	"unpicmpd.h"

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	//icmpfd:连接icmpd服务器程序的Unix域套接字
	//maxfdp1:用于Select函数的最大描述符
	int				icmpfd, maxfdp1;
	char			sendline[MAXLINE], recvline[MAXLINE + 1]; //sendline:发送缓冲区  recvline:接收缓冲区
	fd_set			rset; //用于Senect的描述符集(关心里面那个描述符可读)
	ssize_t			n; //recvfrom读取到的总字节数
	struct timeval	tv; //时间结构体(控制Senect的超时)
	struct icmpd_err icmpd_err; //icmpd_err:服务器将返回这个结构体来指明ICMP的错误信息
	struct sockaddr_un sun; //Unix域套接字地址结构

	Sock_bind_wild(sockfd, pservaddr->sa_family); //用addrinfo查找到的对端地址的地址族,绑定一个临时外出端口

	icmpfd = Socket(AF_LOCAL, SOCK_STREAM, 0); //创建Unix域套接字,用来连接icmpd服务器
	sun.sun_family = AF_LOCAL; //设置Unix域地址结构的地址族
	strcpy(sun.sun_path, ICMPD_PATH); //设置Unix域地址结构的地址路径
	Connect(icmpfd, (SA *)&sun, sizeof(sun)); //连接Unix域套接字到设置好的Unix域地址
	Write_fd(icmpfd, "1", 1, sockfd); //向Unix域套接字(icmpd服务器),发送我们的UDP套接字
	n = Read(icmpfd, recvline, 1); //从Unix域套接字(icmpd服务器)读取回应
	//icmpd服务器会回射一个1,表示操作成功
	if (n != 1 || recvline[0] != '1') 
		//符合此条件判断,说明操作失败
		err_quit("error creating icmp socket, n = %d, char = %c",
				 n, recvline[0]); //打印错误消息并退出程序
	
	//代码走到此处,代表操作成功
	FD_ZERO(&rset); //清空描述符集中的所有描述符
	maxfdp1 = max(sockfd, icmpfd) + 1; //Select最大描述符值,为"UDP套接字描述符"和"Unix域套接字描述符"取大者+1
/* end dgcli011 */

/* include dgcli012 */
	while (Fgets(sendline, MAXLINE, fp) != NULL) { //从文件中读取一行
		//把这一行向UDP套接字中写入(发送)
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen); 
		
		//设置Select的超时时间结构体
		tv.tv_sec = 5; 
		tv.tv_usec = 0;
		
		FD_SET(sockfd, &rset); //把UDP套接字加入描述符集
		FD_SET(icmpfd, &rset); //把Unix域套接字(icmpd服务器)加入描述符集
		//用Select关心Unix域套接字(icmpd服务器)和UDP套接字的可读状态
		if ( (n = Select(maxfdp1, &rset, NULL, NULL, &tv)) == 0) {
			fprintf(stderr, "socket timeout\n"); //超时后打印提示消息
			continue; //继续循环
		}

		if (FD_ISSET(sockfd, &rset)) { //UDP套接字可读
			n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL); //从UDP套接字中读取读取消息
			recvline[n] = 0;	/* null terminate 接收缓冲区缓冲区末尾置0*/
			Fputs(recvline, stdout); //打印从UDP套接字中读取的消息
		}

		if (FD_ISSET(icmpfd, &rset)) { //Unix域套接字(icmpd服务器)描述符可读
			if ( (n = Read(icmpfd, &icmpd_err, sizeof(icmpd_err))) == 0) //读取Unix域套接字(icmpd服务器)回送的消息,存放在icmpd_err结构体中
				//进入此分支代表icmpd服务器已经终止
				err_quit("ICMP daemon terminated");
			else if (n != sizeof(icmpd_err)) //读取到的数据长度不等于icmpd_err结构体的长度
				//分别打印出读取到的长度和icmpd_err结构体的长度,判断出现的问题
				err_quit("n = %d, expected %d", n, sizeof(icmpd_err));
			//代码走到这里说明已经正确读到了icmpd传送过来的icmpd_err结构体
			//打印这个结构体的具体内容
			printf("ICMP error: dest = %s, %s, type = %d, code = %d\n",
				   Sock_ntop((struct sockaddr*)(&icmpd_err.icmpd_dest), icmpd_err.icmpd_len),
				   strerror(icmpd_err.icmpd_errno),
				   icmpd_err.icmpd_type, icmpd_err.icmpd_code);
		}
	}
}
/* end dgcli012 */
