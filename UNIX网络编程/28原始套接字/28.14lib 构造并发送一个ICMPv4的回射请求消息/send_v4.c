#include	"ping.h"

void
send_v4(void)
{
	int			len; //ICMP首部+ICMP数据的总长度
	struct icmp	*icmp; //ICMP首部结构体

	icmp = (struct icmp *) sendbuf; //发送数据缓冲区的开始位置,存放一个ICMP首部
	icmp->icmp_type = ICMP_ECHO; //ICMP消息类型为回射请求
	icmp->icmp_code = 0; //ICMP代码值为0
	icmp->icmp_id = pid; //标识字段设置为本进程PID
	icmp->icmp_seq = nsent++; //序列号为累加的全局变量
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern 填充数据区内容为0xa5(数据区长度定义为datalen)*/
	Gettimeofday((struct timeval *) icmp->icmp_data, NULL); //获得现在的时间点,存放入ICMP请求报文中

	len = 8 + datalen;		/* checksum ICMP header and data 所以这个ICMP报文的总长度是首部长度(8字节)+ICMP数据长度*/
	icmp->icmp_cksum = 0; //ICMP4协议的校验和必须自己计算
	icmp->icmp_cksum = in_cksum((u_short *) icmp, len); //计算ICMP的校验和,存放在ICMP结构体的相应成员中

	Sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen); //发送这个ICMP报文
}
