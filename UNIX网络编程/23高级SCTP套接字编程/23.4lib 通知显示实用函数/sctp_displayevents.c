#include	"unp.h"

void
print_notification(char *notify_buf)
{
	union sctp_notification *snp;
	struct sctp_assoc_change *sac;
	struct sctp_paddr_change *spc;
	struct sctp_remote_error *sre;
	struct sctp_send_failed *ssf;
	struct sctp_shutdown_event *sse;
	struct sctp_adaption_event *ae;
	struct sctp_pdapi_event *pdapi;
	const char *str;
	//转换成联合类型,用这个联合类型中的sn_header结构的sn_type成员来分辨,到达的是什么通知
	snp = (union sctp_notification *)notify_buf;
	switch(snp->sn_header.sn_type) {
	case SCTP_ASSOC_CHANGE: //关联变动通知
		sac = &snp->sn_assoc_change; //把联合中关联改变的信息取出
		switch(sac->sac_state) { //展示各种关联改变状态
		case SCTP_COMM_UP:
			str = "COMMUNICATION UP";
			break;
		case SCTP_COMM_LOST:
			str = "COMMUNICATION LOST";
			break;
		case SCTP_RESTART:
			str = "RESTART";
			break;
		case SCTP_SHUTDOWN_COMP:
			str = "SHUTDOWN COMPLETE";
			break;
		case SCTP_CANT_STR_ASSOC:
			str = "CAN'T START ASSOC";
			break;
		default:
			str = "UNKNOWN";
			break;
		} /* end switch(sac->sac_state) */
		printf("SCTP_ASSOC_CHANGE: %s, assoc=0x%x\n", str,
		       (uint32_t)sac->sac_assoc_id);
		break;
	case SCTP_PEER_ADDR_CHANGE: //对端地址变动通知
		spc = &snp->sn_paddr_change; //把对端地址变化通知的信息取出
		switch(spc->spc_state) { //打印其状态
		case SCTP_ADDR_AVAILABLE:
			str = "ADDRESS AVAILABLE";
			break;
		case SCTP_ADDR_UNREACHABLE:
			str = "ADDRESS UNREACHABLE";
			break;
		case SCTP_ADDR_REMOVED:
			str = "ADDRESS REMOVED";
			break;
		case SCTP_ADDR_ADDED:
			str = "ADDRESS ADDED";
			break;
		case SCTP_ADDR_MADE_PRIM:
			str = "ADDRESS MADE PRIMARY";
			break;
		default:
			str = "UNKNOWN";
			break;
		} /* end switch(spc->spc_state) */
		printf("SCTP_PEER_ADDR_CHANGE: %s, addr=%s, assoc=0x%x\n", str,
		       Sock_ntop((SA *)&spc->spc_aaddr, sizeof(spc->spc_aaddr)),
		       (uint32_t)spc->spc_assoc_id);
		break;
	case SCTP_REMOTE_ERROR: //远程错误
		sre = &snp->sn_remote_error; //把远程错误的信息取出
		printf("SCTP_REMOTE_ERROR: assoc=0x%x error=%d\n",
		       (uint32_t)sre->sre_assoc_id, sre->sre_error); //打印相关信息
		break;
	case SCTP_SEND_FAILED: //发送失败
		//关联正在关闭中,稍后可能会收到关联变动通知
		//服务器再使用部分递送扩展
		ssf = &snp->sn_send_failed; //把发送失败的相关信息取出
		printf("SCTP_SEND_FAILED: assoc=0x%x error=%d\n",
		       (uint32_t)ssf->ssf_assoc_id, ssf->ssf_error); //打印相关信息
		break;
	case SCTP_ADAPTION_INDICATION: //适配层错误
		ae = &snp->sn_adaption_event; //取出适配层错误的相关信息
		printf("SCTP_ADAPTION_INDICATION: 0x%x\n",
		    (u_int)ae->sai_adaption_ind);
		break;
	case SCTP_PARTIAL_DELIVERY_EVENT: //部分递送
	    pdapi = &snp->sn_pdapi_event; //取出部分递送的信息
	    //显示通知事件,目前唯一的事件是部分递送被取消
	    if(pdapi->pdapi_indication == SCTP_PARTIAL_DELIVERY_ABORTED) 
		    printf("SCTP_PARTIAL_DELIEVERY_ABORTED\n");
	    else
		    printf("Unknown SCTP_PARTIAL_DELIVERY_EVENT 0x%x\n",
			   pdapi->pdapi_indication);
	    break;
	case SCTP_SHUTDOWN_EVENT: //关联终止事件
		sse = &snp->sn_shutdown_event; //取出关联终止事件的相关信息
		//表示对端已经发送了一个SHUTDOWN消息,到关联终止序列完成时,会收到一个关联变动通知
		printf("SCTP_SHUTDOWN_EVENT: assoc=0x%x\n",
		       (uint32_t)sse->sse_assoc_id);
		break;
	default:
		printf("Unknown notification event type=0x%x\n", 
		       snp->sn_header.sn_type);
	}
}
