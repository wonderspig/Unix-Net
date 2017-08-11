#include	"unp.h"

void
check_notification(int sock_fd,char *recvline,int rd_len)
{
	union sctp_notification *snp;
	struct sctp_assoc_change *sac;
	struct sockaddr_storage *sal,*sar;
	int num_rem, num_loc;
	//因为是通知,所以把通知转换成通用通知格式
	snp = (union sctp_notification *)recvline; 
	if(snp->sn_header.sn_type == SCTP_ASSOC_CHANGE) { //判断类型是否为关联变动
		sac = &snp->sn_assoc_change; //从联合中取出关联变动的信息
		if((sac->sac_state == SCTP_COMM_UP) ||
		   (sac->sac_state == SCTP_RESTART)) { //查看是否是一个新的(SCTP_COMM_UP),或重新激活的(SCTP_RESTART)关联
		   	//如果是,则调用sctp_getpaddrs获得关联的对端地址
		   	//从sock_fd套接字中,关联ID是sac->sac_assoc_id,获得的对端地址填写在sar中,sar中的地址数目是返回值
			num_rem = sctp_getpaddrs(sock_fd,sac->sac_assoc_id,(SA**)&sar);
			printf("There are %d remote addresses and they are:\n",
			       num_rem);
			sctp_print_addresses(sar,num_rem); //地址显示实用函数
			sctp_freepaddrs((SA*)sar); //释放sctp_getpaddrs函数产生的空间

			//从sock_fd套接字中,关联ID是sac->sac_assoc_id,获得的本地地址填写在sal中,sal中的地址数目是返回值
			num_loc = sctp_getladdrs(sock_fd,sac->sac_assoc_id,(SA**)&sal);
			printf("There are %d local addresses and they are:\n",
			       num_loc);
			sctp_print_addresses(sal,num_loc);//地址显示实用函数
			sctp_freeladdrs((SA*)sal); //释放sctp_getladdrs函数产生的空间
		}
	}

}
