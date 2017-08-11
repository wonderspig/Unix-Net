#include	"unp.h"

void
sctpstr_cli(FILE *fp, int sock_fd, struct sockaddr *to, socklen_t tolen)
{
	struct sockaddr_in peeraddr;
	struct sctp_sndrcvinfo sri; //因为关心了事件,这个结构体会在调用sctp_recvmsg时填写
	char sendline[MAXLINE], recvline[MAXLINE];
	socklen_t len;
	int out_sz,rd_sz;
	int msg_flags;

	bzero(&sri,sizeof(sri)); //清空sri结构体
	while (fgets(sendline, MAXLINE, fp) != NULL) { //从标准输入fp中读入一行
		if(sendline[0] != '[') { //如果读入的一行不是以'['开头,打印错误提示消息,且继续循环
			printf("Error, line must be of the form '[streamnum]text'\n");
			continue;
		}
		//把方括号中的数组转换成数值,并附值给sri.sinfo_stream,服务器会sri.sinfo_stream(流的值)加一
		sri.sinfo_stream = strtol(&sendline[1],NULL,0); 
		out_sz = strlen(sendline); //计算一下输出的字节数
		//把从标准输入读到的字符串传递给服务器(sock_fd),其中发送参数:净荷标示符,标志,生命周期,上下文都为NULL
		Sctp_sendmsg(sock_fd, sendline, out_sz, 
			     to, tolen, 
			     0, 0,
			     sri.sinfo_stream,
			     0, 0); 

		len = sizeof(peeraddr); //计算一下对端(服务器端)地址结构体的大小
		//从服务器端接收数据,存放在recvline中,并将服务器地址天填写在peeraddr
		//根据关心的事件,还会填写sri这个结构题,里面是服务端发送的参数
		rd_sz = Sctp_recvmsg(sock_fd, recvline, sizeof(recvline),
			     (SA *)&peeraddr, &len,
			     &sri,&msg_flags); 
		printf("From str:%d seq:%d (assoc:0x%x):",
		       sri.sinfo_stream,sri.sinfo_ssn,
		       (u_int)sri.sinfo_assoc_id); //打印默认流,流序号,关联标识
		printf("%.*s",rd_sz,recvline); //打印接收到的字节数,和接收到的具体文本消息
	}
}
