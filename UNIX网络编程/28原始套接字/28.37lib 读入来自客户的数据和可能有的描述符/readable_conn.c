/* include readable_conn1 */
#include	"icmpd.h"

int
readable_conn(int i)
{
	int				unixfd, recvfd;
	char			c;
	ssize_t			n;
	socklen_t		len;
	struct sockaddr_storage	ss;

	unixfd = client[i].connfd; //拿出保存在client结构体数组中的连接到客户的Unix域套接字
	recvfd = -1; //接收客户进程传过来的,已经绑定好端口号的,UDP套接字
	//从连接到客户进程的Unix域套接字中,读取客户进程传递过来的已经绑定好端口号的,UDP套接字
	if ( (n = Read_fd(unixfd, &c, 1, &recvfd)) == 0) { 
		//进入此分支,代表客户进程已经终止
		err_msg("client %d terminated, recvfd = %d", i, recvfd); //打印错误消息后,跳转到clientdone进行处理
		goto clientdone;	/* client probably terminated 跳转到clientdone进行处理(客户既然已经终止,已无必要再往客户端回写一个0,表示失败了) */
	}

		/* 4data from client; should be descriptor */
	if (recvfd < 0) {
		//进入此分支代表没有接收到客户进程传递过来的UDP套接字描述符,跳转到clienterr进行处理
		err_msg("read_fd did not return descriptor");
		goto clienterr; //跳转到clienterr进行处理
	}
/* end readable_conn1 */

/* include readable_conn2 */
	len = sizeof(ss); //计算一下sockaddr_storage套接字地址结构体的长度(此结构体能保存IP4和IP6地址)
	if (getsockname(recvfd, (SA *) &ss, &len) < 0) { //获得客户进程绑定在UDP套接字上的地址结构体
		err_ret("getsockname error");
		goto clienterr;
	}
	
	//从客户绑定在UDP套接字上的地址结构体中取出地址族信息,保存在client结构体数组中已找到的可用元素的对应成员中
	client[i].family = ss.ss_family; 
	//从客户绑定在UDP套接字上的地址结构体中取出端口号信息,保存在client结构体数组中已找到的可用元素的对应成员中
	if ( (client[i].lport = sock_get_port((SA *)&ss, len)) == 0) {
		//进入此分支代表从客户绑定在UDP套接字上的地址结构体中取出端口号端口号为0
		client[i].lport = sock_bind_wild(recvfd, client[i].family); //我们使用通配地址和临时端口号绑定到这个套接字(SVR4实现上行不通)
		if (client[i].lport <= 0) { ////从客户绑定在UDP套接字上的地址结构体中取出端口号小于0
			err_ret("error binding ephemeral port"); //那是错误的端口号
			goto clienterr; //跳转到clientdone进行处理
		}
	}
	Write(unixfd, "1", 1);	/* tell client all OK 往客户进程连接的套接字回写一个1,代表处理成功 */
	Close(recvfd);			/* all done with client's UDP socket 关闭客户传递过来的UPD套接字 */
	return(--nready); //待处理的描述符个数-1,函数返回

clienterr:
	Write(unixfd, "0", 1);	/* tell client error occurred 网客户端回写一个0,表示操作失败*/
clientdone:
	Close(unixfd); //关闭连接到客户进程的Unix域套接字
	if (recvfd >= 0) //如果已经接收到了客户进程传递过来的UDP套接字,则关闭这个套接字
		Close(recvfd); //关闭客户传递过来的UDP套接字
	FD_CLR(unixfd, &allset); //把连接到这个客户进程的Unix套接字从Select的描述符集中删除
	client[i].connfd = -1; //client结构体数组中这个客户所使用的那个元素,重新标记为可用
	return(--nready); //待处理的描述符个数-1,函数返回
}
/* end readable_conn2 */
