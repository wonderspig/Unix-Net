/* include getrt1 */
#include	"unproute.h"

#define	BUFLEN	(sizeof(struct rt_msghdr) + 512)
					/* sizeof(struct sockaddr_in6) * 8 = 192 */
#define	SEQ		9999

int
main(int argc, char **argv)
{
	int					sockfd;
	char				*buf;
	pid_t				pid;
	ssize_t				n;
	struct rt_msghdr	*rtm;
	struct sockaddr		*sa, *rti_info[RTAX_MAX];
	struct sockaddr_in	*sin;

	if (argc != 2) //参数不为1,则出错退出
		err_quit("usage: getrt <IPaddress>");

	sockfd = Socket(AF_ROUTE, SOCK_RAW, 0);	/* need superuser privileges 创建套接字,路由域套接字*/

	buf = Calloc(1, BUFLEN);	/* and initialized to 0 创建一个缓冲区,初始化为0 */

	rtm = (struct rt_msghdr *) buf; //把buf强转成rt_msghdr结构
	//本消息的长度为rt_msghdr结构体的长度+IP4地址结构体的长度
	rtm->rtm_msglen = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_in);
	rtm->rtm_version = RTM_VERSION; //版本
	rtm->rtm_type = RTM_GET; //类型
	rtm->rtm_addrs = RTA_DST; //数位掩码:指明本消息后跟随的套接字地址结构是什么类型(目的地址)
	rtm->rtm_pid = pid = getpid(); //本进程pid
	rtm->rtm_seq = SEQ; //确定发送者

	//rtm指针向后移动sizeof(struct rt_msghdr)个字节,也就是移动到了第一个套接字地址结构的开始处
	//所以强转成IP4地址没有任何问题
	sin = (struct sockaddr_in *) (rtm + 1); 
	sin->sin_len = sizeof(struct sockaddr_in); //IP4地址结构的长度
	sin->sin_family = AF_INET; //IP4地址族
    Inet_pton(AF_INET, argv[1], &sin->sin_addr); //把命令行第一个参数转换成二进制格式,填入地址结构体
    //rtm结构体填写完成,把它写入套接字,传送给内核
	Write(sockfd, rtm, rtm->rtm_msglen);

	do {
		n = Read(sockfd, rtm, BUFLEN); //从套接字中读取内核的回应
	} while (rtm->rtm_type != RTM_GET || rtm->rtm_seq != SEQ ||
			 rtm->rtm_pid != pid); //筛选出符合我们需要的信息
/* end getrt1 */

/* include getrt2 */
	rtm = (struct rt_msghdr *) buf; //让rtm重新指向rt_msghdr结构的开始处
	sa = (struct sockaddr *) (rtm + 1); //让sa指向第一个套接字地址结构
	get_rtaddrs(rtm->rtm_addrs, sa, rti_info); //构造套接字地址结构的指针数组,存放入rti_info指针数组中
	if ( (sa = rti_info[RTAX_DST]) != NULL) //ti_info指针数组中取出目的地址的指针
		printf("dest: %s\n", Sock_ntop_host(sa, sa->sa_len));

	if ( (sa = rti_info[RTAX_GATEWAY]) != NULL) //ti_info指针数组中取出网关地址的指针
		printf("gateway: %s\n", Sock_ntop_host(sa, sa->sa_len));

	if ( (sa = rti_info[RTAX_NETMASK]) != NULL) //ti_info指针数组中取出子网掩码地址的指针
		printf("netmask: %s\n", Sock_masktop(sa, sa->sa_len)); 

	if ( (sa = rti_info[RTAX_GENMASK]) != NULL) //ti_info指针数组中取出克隆掩码地址的指针
		printf("genmask: %s\n", Sock_masktop(sa, sa->sa_len)); 

	exit(0);
}
/* end getrt2 */
