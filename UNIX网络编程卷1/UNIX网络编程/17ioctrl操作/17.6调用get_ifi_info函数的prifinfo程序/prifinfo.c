#include	"unpifi.h"

int
main(int argc, char **argv)
{
	struct ifi_info	*ifi, *ifihead;
	struct sockaddr	*sa;
	u_char			*ptr;
	int				i, family, doaliases;

	if (argc != 3) //如果参数不等于2,则出错
		err_quit("usage: prifinfo <inet4|inet6> <doaliases>");

	if (strcmp(argv[1], "inet4") == 0) //看看是不是指定ip4参数
		family = AF_INET; //地址族为IP4
#ifdef	IPv6 //如果主机支持ip6
	else if (strcmp(argv[1], "inet6") == 0) //命令行参数指定了IP6
		family = AF_INET6; //地址族设置为IP6
#endif
	else
		err_quit("invalid <address-family>"); //都不是就是无效的地址族参数
	doaliases = atoi(argv[2]); //是否返回地址别名

	//用确定好的地址族,是否返回别名为参数,调用Get_ifi_info,返回一个ifi_info结构体链表,且指向头节点
	for (ifihead = ifi = Get_ifi_info(family, doaliases); 
		 ifi != NULL; ifi = ifi->ifi_next) { //用玄幻遍历这个链表
		printf("%s: ", ifi->ifi_name); //打印接口名字
		if (ifi->ifi_index != 0)
			printf("(%d) ", ifi->ifi_index); //打印借口索引
		printf("<");
/* *INDENT-OFF* */
		if (ifi->ifi_flags & IFF_UP)			printf("UP "); //接口是否启用
		if (ifi->ifi_flags & IFF_BROADCAST)		printf("BCAST "); //是否支持广播
		if (ifi->ifi_flags & IFF_MULTICAST)		printf("MCAST ");
		if (ifi->ifi_flags & IFF_LOOPBACK)		printf("LOOP ");
		if (ifi->ifi_flags & IFF_POINTOPOINT)	printf("P2P ");
		printf(">\n");
/* *INDENT-ON* */

		if ( (i = ifi->ifi_hlen) > 0) { //如果返回了硬件地址
			ptr = ifi->ifi_haddr;
			do { //打印硬件地址的相关信息
				printf("%s%x", (i == ifi->ifi_hlen) ? "  " : ":", *ptr++);
			} while (--i > 0);
			printf("\n");
		}
		if (ifi->ifi_mtu != 0)
			printf("  MTU: %d\n", ifi->ifi_mtu); //打印MTU信息

		if ( (sa = ifi->ifi_addr) != NULL)
			printf("  IP addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa))); //打印主地址信息
		if ( (sa = ifi->ifi_brdaddr) != NULL) 
			printf("  broadcast addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa))); //打印广播地址信息
		if ( (sa = ifi->ifi_dstaddr) != NULL)
			printf("  destination addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa))); //打印点到点链路的目的地址
	}
	free_ifi_info(ifihead); //释放Get_ifi_info所分配的空间
	exit(0);
}
