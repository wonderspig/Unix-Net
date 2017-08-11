#include	"unp.h"

int
main(int argc, char **argv)
{
	int sock_fd,msg_flags,connfd,childpid;
	sctp_assoc_t assoc;
	char readbuf[BUFFSIZE];
	struct sockaddr_in servaddr, cliaddr;
	struct sctp_sndrcvinfo sri;
	struct sctp_event_subscribe evnts;
	socklen_t len;
	size_t rd_sz;

    sock_fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP); //创建IP4套接字,SCTP
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(sock_fd, (SA *) &servaddr, sizeof(servaddr));
	
	bzero(&evnts, sizeof(evnts));
	evnts.sctp_data_io_event = 1;
	Setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS,
		   &evnts, sizeof(evnts));

	Listen(sock_fd, LISTENQ);
/* include mod_servfork */
	for ( ; ; ) {
		len = sizeof(struct sockaddr_in); //计算IP4地址族地址结构体的大小
		//接收一个客户请求:从sock_fd套接字接收消息,发送端地址存放在cliaddr中
		//开启事件接收发送端信息,填写sri结构体,标志存放在msg_flags中
		rd_sz = Sctp_recvmsg(sock_fd, readbuf, sizeof(readbuf),
			     (SA *)&cliaddr, &len,
			     &sri,&msg_flags);
		//回射客户消息:发送到套接字sock_fd,发送内容为readbuf,发送地址为cliaddr
		//净荷标识,标志,流全部不变的回送给客户,超时时间和上下文为null
		Sctp_sendmsg(sock_fd, readbuf, rd_sz, 
			     (SA *)&cliaddr, len,
			     sri.sinfo_ppid,
			     sri.sinfo_flags,
			     sri.sinfo_stream,
			     0, 0);
		assoc = sctp_address_to_associd(sock_fd,(SA *)&cliaddr,len, sri.sinfo_assoc_id); //通过对端地址获得关联标识符
		if((int)assoc == 0){ //获取标示符失败,则退出
			err_ret("Can't get association id");
			continue;
		} 
		connfd = sctp_peeloff(sock_fd,assoc); //通过和客户关联的标示符剥离出一个和客户一对一的关联
		if(connfd == -1){ //剥离失败
			err_ret("sctp_peeloff fails");
			continue;
		}
		if((childpid = fork()) == 0) { //创建子进程(在子进程中)
			Close(sock_fd); //子进程中关闭一到多套接字(类似于监听套接字)
			str_echo(connfd); //把与客户关联的一对一的套接字传参进去
			exit(0);
		} else {
			Close(connfd); //父进程中关闭与客户链接的一到一套接字
		}
	}
/* end mod_servfork */
}
