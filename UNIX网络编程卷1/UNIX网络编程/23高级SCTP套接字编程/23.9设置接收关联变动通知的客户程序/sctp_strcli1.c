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
		out_sz = strlen(sendline);
		Sctp_sendmsg(sock_fd, sendline, out_sz, 
			     to, tolen, 
			     0, 0,
			     sri.sinfo_stream,
			     0, 0);
/* include mod_strcli1 */		
		do {
			len = sizeof(peeraddr); //计算对端地址结构体的空间
			//接受消息:从sock_fd接受消息,存放在recvline中,发送端地址填写在peeraddr中,
			//因为关注了事件,接收sctp发送端选项,填写结构体sri,标志填写在msg_flags中
			rd_sz = Sctp_recvmsg(sock_fd, recvline, sizeof(recvline),
				     (SA *)&peeraddr, &len,
				     &sri,&msg_flags);
			if(msg_flags & MSG_NOTIFICATION) //判断是否有通知到达
				//有通知到达就用check_notification处理
				check_notification(sock_fd,recvline,rd_sz);
		} while (msg_flags & MSG_NOTIFICATION); //循环到不是通知事件的消息为止
		printf("From str:%d seq:%d (assoc:0x%x):",
		       sri.sinfo_stream,sri.sinfo_ssn,
		       (u_int)sri.sinfo_assoc_id);
		printf("%.*s",rd_sz,recvline);
/* end mod_strcli1 */		
	}
}
