#include	"sntp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	char				buf[MAXLINE];
	ssize_t				n;
	socklen_t			salen, len;
	struct ifi_info		*ifi;
	struct sockaddr		*mcastsa, *wild, *from;
	struct timeval		now;

	if (argc != 2) //如果命令行有一个参数
		err_quit("usage: ssntp <IPaddress>");
	//创建套接字,主机名为命令行参数,端口号为"ntp"的端口号,把套接字地址填写在mcastsa中
	sockfd = Udp_client(argv[1], "ntp", (void **) &mcastsa, &salen);

	wild = Malloc(salen); //为地址分配空间
	memcpy(wild, mcastsa, salen);	/* copy family and port 拷贝地址到刚才分配的空间中*/
	sock_set_wild(wild, salen); //设置wild结构的ip地址为通配ip地址
	Bind(sockfd, wild, salen);	/* bind wildcard 把这个地址绑定到UDP套接字上*/

#ifdef	MCAST
		/* 4obtain interface list and process each one */
	//Get_ifi_info函数返回所有的接口和地址信息,返回值指向链表头节点
	//查询的地址族取自命令行参数确定的地址结构中的地址族
	//这个循环用来遍历这个链表
	for (ifi = Get_ifi_info(mcastsa->sa_family, 1); ifi != NULL;
		 ifi = ifi->ifi_next) {
		if (ifi->ifi_flags & IFF_MULTICAST) { //查看这个接口知否具有多播能力
			//如果这个接口具有多播能力
			//在套接字sockfd上面,以mcastsa为目的多播地址,当前接口的名字为参数,加入该多播组
			Mcast_join(sockfd, mcastsa, salen, ifi->ifi_name, 0);
			//打印一下已经加入的多播组信息
			//Sock_ntop函数:协议无关的转换二进制地址格式为表达式格式
			printf("joined %s on %s\n",
				   Sock_ntop(mcastsa, salen), ifi->ifi_name);
		}
	}
#endif

	from = Malloc(salen); //分配套接字地址结构体空间
	for ( ; ; ) {
		//从多播套接字中收取UDP数据报
		len = salen;
		n = Recvfrom(sockfd, buf, sizeof(buf), 0, from, &len);
		Gettimeofday(&now, NULL); //获得当前时间
		sntp_proc(buf, n, &now); //sntp_proc函数计算时间差
	}
}
