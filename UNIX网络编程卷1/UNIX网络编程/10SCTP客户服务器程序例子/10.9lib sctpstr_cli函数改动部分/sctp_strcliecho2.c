#include	"unp.h"

#define	SCTP_MAXLINE	800
#define SERV_MAX_SCTP_STRM 10

void
sctpstr_cli_echoall(FILE *fp, int sock_fd, struct sockaddr *to, socklen_t tolen)
{
	struct sockaddr_in peeraddr;
	struct sctp_sndrcvinfo sri;
	char sendline[SCTP_MAXLINE], recvline[SCTP_MAXLINE];
	socklen_t len;
	int rd_sz,i,strsz;
	int msg_flags;

	bzero(sendline,sizeof(sendline)); //初始化清空接受缓冲区
	bzero(&sri,sizeof(sri)); //初始化清空发送参数结构体
	while (fgets(sendline, SCTP_MAXLINE - 9, fp) != NULL) { //从标准输入fp读入一行
		strsz = strlen(sendline); //计算一下发送缓冲区的字符个数
		if(sendline[strsz-1] == '\n') { //如果是以'\n'结尾
			sendline[strsz-1] = '\0'; //那么把'\n'变成'\0'
			strsz--; //替换完成后,字符串个数自然要-1
		}
/* include modified_echo */
		for(i=0;i<SERV_MAX_SCTP_STRM;i++) {
			snprintf(sendline + strsz, sizeof(sendline) - strsz,
				".msg.%d 1", i); //在缓冲区的末尾添加本条消息的流号
			//改动部分:向同一个流中写入两条消息,并且标明序号
			Sctp_sendmsg(sock_fd, sendline, sizeof(sendline), 
				     to, tolen, 
				     0, 0,
				     i,
				     0, 0); //向服务器sock_fd发送消息sendline,发送参数只关心流号,其他置空
			snprintf(sendline + strsz, sizeof(sendline) - strsz,
				".msg.%d 2", i);
			Sctp_sendmsg(sock_fd, sendline, sizeof(sendline), 
				     to, tolen, 
				     0, 0,
				     i,
				     0, 0);
		}
		for(i=0;i<SERV_MAX_SCTP_STRM*2;i++) { //用循环依次接受每个流中到达的数据
			len = sizeof(peeraddr); //计算一下对端服务器端结构体地址的大小
/* end modified_echo */
			//从服务器sock_fd中接受消息,存放在recvline中,填写服务器地址结构体peeraddr,
			//关心对端服务器的发送参数结构体sri
			rd_sz = Sctp_recvmsg(sock_fd, recvline, sizeof(recvline),
				     (SA *)&peeraddr, &len,
				     &sri,&msg_flags); 
			printf("From str:%d seq:%d (assoc:0x%x):",
				sri.sinfo_stream,sri.sinfo_ssn,
				(u_int)sri.sinfo_assoc_id);  //打印收到消息的流号,流序号,关联id
			printf("%.*s\n",rd_sz,recvline); //打印接收到的字节数,和接收到的具体文本消息
		}
	}
}
