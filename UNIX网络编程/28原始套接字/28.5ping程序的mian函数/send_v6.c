#include	"ping.h"

void
send_v6()
{
#ifdef	IPV6
	int					len; //ICMP首部+ICMP数据的总长度
	struct icmp6_hdr	*icmp6; //ICMP首部结构体

	icmp6 = (struct icmp6_hdr *) sendbuf; //发送数据缓冲区的开始位置,存放一个ICMP首部
	icmp6->icmp6_type = ICMP6_ECHO_REQUEST; //ICMP消息类型为回射请求
	icmp6->icmp6_code = 0; //ICMP代码值为0
	icmp6->icmp6_id = pid; //标识字段设置为本进程PID
	icmp6->icmp6_seq = nsent++; //序列号为累加的全局变量
	memset((icmp6 + 1), 0xa5, datalen);	/* fill with pattern 填充数据区内容为0xa5(数据区长度定义为datalen)*/
	Gettimeofday((struct timeval *) (icmp6 + 1), NULL); //获得现在的时间点,存放入ICMP请求报文中

	/* 这个ICMP报文的总长度是首部长度(8字节)+ICMP数据长度*/
	len = 8 + datalen;		/* 8-byte ICMPv6 header */

	Sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen); //发送这个ICMP报文
		/* 4kernel calculates and stores checksum for us */
#endif	/* IPV6 */
}
