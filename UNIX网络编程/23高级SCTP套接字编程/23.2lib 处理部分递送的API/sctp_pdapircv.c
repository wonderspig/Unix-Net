#include	"unp.h"

static uint8_t *sctp_pdapi_readbuf=NULL;
static int sctp_pdapi_rdbuf_sz=0;

#define SCTP_PDAPI_INCR_SZ 4096
#define SCTP_PDAPI_NEED_MORE_THRESHOLD 1024

uint8_t *
pdapi_recvmsg(int sock_fd,
	      int *rdlen,
	      SA *from,
	      int *from_len,
	      struct sctp_sndrcvinfo *sri,
	      int *msg_flags)
{
	int rdsz,left,at_in_buf;
	int frmlen=0;
	//如果全局静态指针指向的接收缓冲区尚未分配,那么就分配其大小,并设置与它关联的状态
	if (sctp_pdapi_readbuf == NULL) {
		sctp_pdapi_readbuf = (uint8_t *)Malloc(SCTP_PDAPI_INCR_SZ); //分配静态接收缓冲区的大小
		sctp_pdapi_rdbuf_sz = SCTP_PDAPI_INCR_SZ; //设置其关联的状态(缓冲区大小)
	}
	//调用Sctp_recvmsg读入一条消息
	//从sock_fd套接字中读,存放在缓冲区sctp_pdapi_readbuf中,发送者的地址存放在from中
	//msg_flags中存放消息标志,如果启用sctp_data_io_event标志,相关消息存放在sri结构体中
	at_in_buf = Sctp_recvmsg(sock_fd, sctp_pdapi_readbuf, sctp_pdapi_rdbuf_sz,
				 from, from_len,
				 sri,msg_flags);
	if(at_in_buf < 1){ //读到EOF,或者错误,直接返回给调用者
		*rdlen = at_in_buf; //直接返回给调用者
		return(NULL);
	}
	while((*msg_flags & MSG_EOR) == 0) { //MSG_EOR宏表示收到的是不完整的消息,则继续手机其余断片
		left = sctp_pdapi_rdbuf_sz - at_in_buf; //缓冲区的总大小-当前已经使用的大小=还剩余的缓冲区大小
		if(left < SCTP_PDAPI_NEED_MORE_THRESHOLD) { //当缓冲区大小小于一个最小量的时候
			//realloc调整缓冲区大小
			sctp_pdapi_readbuf = realloc(sctp_pdapi_readbuf, sctp_pdapi_rdbuf_sz+SCTP_PDAPI_INCR_SZ);
			if(sctp_pdapi_readbuf == NULL) { //调整失败
				err_quit("sctp_pdapi ran out of memory");
			}
			sctp_pdapi_rdbuf_sz += SCTP_PDAPI_INCR_SZ; //计算新的缓冲区大小
			left = sctp_pdapi_rdbuf_sz - at_in_buf; //计算还剩余的缓冲区大小
		}
		//调用Sctp_recvmsg继续接收其余的断片
		//从sock_fd套接字接收,存放在缓冲区sctp_pdapi_readbuf中,从at_in_buf字节开始就存放
		//不关心发送者协议地址,不关心sctp的相关信息
		//发送者的协议地址大小存放在frmlen中,发送者标志存放在msg_flags中
		rdsz = Sctp_recvmsg(sock_fd, &sctp_pdapi_readbuf[at_in_buf], 
			     left, NULL, &frmlen, NULL, msg_flags);
		at_in_buf += rdsz; //更新下标索引,用来指示下一次断片再缓冲区中存放的位置
	}
	//已经读好了一个完整的消息到缓冲区中
	*rdlen = at_in_buf; //把一共读到的字节数返回给调用者
	return(sctp_pdapi_readbuf); //把接收缓冲区的指针返回给调用者
}
