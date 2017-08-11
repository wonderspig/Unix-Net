#include	"unpifi.h"
#include	<net/if_arp.h>

int
main(int argc, char **argv)
{
	int					sockfd;
	struct ifi_info			*ifi;
	char		*ptr;
	struct arpreq		arpreq;
	struct sockaddr_in	*sin;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建一个IP4,UDP套接字
	//get_ifi_info函数返回一个ifi_info链表的头节点
	//循环为每一个链表节点获得arpreq(arp硬件地址)
	for (ifi = get_ifi_info(AF_INET, 0); ifi != NULL; ifi = ifi->ifi_next) {
		//打印地址
		//Sock_ntop把二进制地址转换成表达式格式(无关协议)
		printf("%s: ", Sock_ntop(ifi->ifi_addr, sizeof(struct sockaddr_in)));

		sin = (struct sockaddr_in *) &arpreq.arp_pa; //强转成ip4格式的地址
		memcpy(sin, ifi->ifi_addr, sizeof(struct sockaddr_in)); //把链表节点中的地址复制到sin中

		if (ioctl(sockfd, SIOCGARP, &arpreq) < 0) { //让ioctl填写arpreq
			err_ret("ioctl SIOCGARP");
			continue; 
		}

		ptr = &arpreq.arp_ha.sa_data[0]; //ptr指向硬件地址缓冲区的开始处
		printf("%x:%x:%x:%x:%x:%x\n", *ptr, *(ptr+1),
			   *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5)); //打印硬件地址
	}
	exit(0);
}
