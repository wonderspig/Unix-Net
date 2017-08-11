#include	"unp.h"

void
sctpstr_cli(FILE *fp, int sock_fd, struct sockaddr *to, socklen_t tolen)
{
	struct sockaddr_in peeraddr;
	struct sctp_sndrcvinfo sri;
	char sendline[MAXLINE], recvline[MAXLINE];
	socklen_t len;
	int out_sz,rd_sz;
	int msg_flags;

	bzero(&sri,sizeof(sri));
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		if(sendline[0] != '[') {
			printf("Error, line must be of the form '[streamnum]text'\n");
			continue;
		}
		sri.sinfo_stream = strtol(&sendline[1],NULL,0);
/* include mod_unordered */
		out_sz = strlen(sendline); //计算发送缓冲区大小字节数
		//发送数据报:往套接字sock_fd上发送数据报,发送内容为sendline,目的地址to
		//第六个参数为净荷标示符
		//标志设定为MSG_UNORDERED无序,发送的流为sinfo_stream
		//无限的生命周期,上下文关联为NULL
		Sctp_sendmsg(sock_fd, sendline, out_sz, 
			     to, tolen, 
			     0,
			     MSG_UNORDERED,
			     sri.sinfo_stream,
			     0, 0);
/* end mod_unordered */
		len = sizeof(peeraddr);
		rd_sz = Sctp_recvmsg(sock_fd, recvline, sizeof(recvline),
			     (SA *)&peeraddr, &len,
			     &sri,&msg_flags);
		printf("From str:%d seq:%d (assoc:0x%x):",
		       sri.sinfo_stream,sri.sinfo_ssn,
		       (u_int)sri.sinfo_assoc_id);
		printf("%.*s",rd_sz,recvline);
	}
}
