#include	"unp.h"

void
sctp_print_addresses(struct sockaddr_storage *addrs, int num)
{
	struct sockaddr_storage *ss;
	int i,salen;

	ss = addrs;
	for(i=0; i<num; i++){ //根据调用者指定的地址数目遍历每一个地址
		printf("%s\n", Sock_ntop((SA *)ss, salen)); //打印地址信息

//sctp_getpaddrs和sctp_getladdrs返回的地址列表是紧凑的,并不是一个个的sockaddr_storage数组
//所以要确定每一个地址的长度,才能确定让指针往后走多少才能读到第二个地址

#ifdef HAVE_SOCKADDR_SA_LEN //HAVE_SOCKADDR_SA_LEN宏表示套接字实现有地址长度成员
		salen = ss->ss_len; //把地址结构的长度提取出来
#else
		switch(ss->ss_family) { //否则就根据协议族来确定地址结构的长度
		case AF_INET:
			salen = sizeof(struct sockaddr_in);
			break;
#ifdef IPV6
		case AF_INET6:
			salen = sizeof(struct sockaddr_in6);
			break;
#endif
		default:
			err_quit("sctp_print_addresses: unknown AF");
			break;
		}
#endif
		//确定下一个地址的为止
		//ss强转成char*(让ss指针以字节为单位移动),然后加上salen(salen是确定的地址的长度)
		//此时ss已经指向了第二个地址,再强转成sockaddr_storage,就可以访问了
		ss = (struct sockaddr_storage *)((char *)ss + salen);
	}
}
