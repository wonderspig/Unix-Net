#include	"unp.h"
#include	<netinet/sctp.h>

int
main(int argc, char **argv)
{
	int sock_fd,msg_flags;
	char readbuf[BUFFSIZE];
	struct sockaddr_in servaddr, cliaddr;
	struct sctp_sndrcvinfo sri; //开启sctp_data_io_event通知事件后,会填写此结构体
	struct sctp_event_subscribe evnts; //通过这个结构体,可以预定8类事件的通知
	int stream_increment=1;
	socklen_t len;
	size_t rd_sz;

	if (argc == 2) //如果有一个命令行参数
		stream_increment = atoi(argv[1]); //把字符串转换成int
        sock_fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP); //创建套接字ip4地址族,sctp协议
	bzero(&servaddr, sizeof(servaddr)); //初始化清空地址结构体
	servaddr.sin_family = AF_INET; //ip4地址族
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定通配ip地址
	servaddr.sin_port = htons(SERV_PORT); //绑定端口号

	Bind(sock_fd, (SA *) &servaddr, sizeof(servaddr)); //绑定填写好的套接字地址结构
	
	bzero(&evnts, sizeof(evnts)); //初始化清空通知结构体
	evnts.sctp_data_io_event = 1; //开启填写sctp_sndrcvinfo这个结构体的通知事件
	Setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS,
		   &evnts, sizeof(evnts)); //把这个SCTP_EVENTS套接字选项设置进sock_fd套接字中

	Listen(sock_fd, LISTENQ); //监听这个套接字
	for ( ; ; ) {
		len = sizeof(struct sockaddr_in); //计算地址结构体长度,用来调用Sctp_recvmsg函数
		//从sock_fd套接字中读,内容存放入readbuf中,地址填写在cliaddr中
		//关于事件的开启,发送端的发送选项则填写sri这个结构体
		rd_sz = Sctp_recvmsg(sock_fd, readbuf, sizeof(readbuf),
			     (SA *)&cliaddr, &len,
			     &sri,&msg_flags);
		if(stream_increment) { //如果命令行参数输入了地址
			sri.sinfo_stream++; //则流号增加1
			//此函数可以获得发送到对端的最大的流的数目
			if(sri.sinfo_stream >= sctp_get_no_strms(sock_fd,(SA *)&cliaddr, len, sri.sinfo_assoc_id)) 
				sri.sinfo_stream = 0; //如果自加之后的流比最大的流还要大,那么把流置为0
		}
		//把从客户端接收到的内容readbuf发送给客户端,客户端地址为cliaddr,净荷标示符,标志都从sri结构体中获得
		//流号自加1,生命周期为无限,用户上下文为空
		Sctp_sendmsg(sock_fd, readbuf, rd_sz,
			     (SA *)&cliaddr, len,
			     sri.sinfo_ppid,
			     sri.sinfo_flags,
			     sri.sinfo_stream,
			     0, 0);
	}
}
