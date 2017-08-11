#include	"unproute.h"

//路由套接字返回的套接字结构地址是可变的,这段代码假设每个套接字地址结构有一个指明自身长度的sa_len成员
//网络掩码和克隆掩码可能通过sa_len为0的套接字地址结构返回,然而实际上占用一个u_long地址的大小

/*
 * Round up 'a' to next multiple of 'size', which must be a power of 2
 */
 //ROUNDUP宏用来计算出当sa_len不为0时,指针应该往后走多少,可以到达下一个地址结构体的开始处
 //ROUNDUP宏可以计算出a沿着size的大小为倍数向上取整
 //此例:a是sa_len(通用套接字地址结构sockaddr的成员),size是u_long,size的大小为8,size-1=7,转换成二进制位为111
 //如果a的最后三个二进制位是000,则是size的倍数,如果最后三个二进制位不是000,则不是size的倍数
 //a & size-1,如果此条件为false,则代表a的最后三个二进制位是000,a本来就是size的倍数,直接返回a
 //a & size-1,如果此条件true,则代表a的最后三个二进制位不是000,a | size-1后最后三位变成111,(a | size-1)+1后,最后三个二进制位变成000
 //成为了size的倍数
#define ROUNDUP(a, size) (((a) & ((size)-1)) ? (1 + ((a) | ((size)-1))) : (a))

/*f
 * Step to next socket address structure;
 * if sa_len is 0, assume it is sizeof(u_long).
 */
 //如果sa_len为0,则指针ap向后移动sizeof(unsigned long)
 //如果sa_len不为0,则通过ROUNDUP宏,返回sa_len沿着sizeof(unsigned long)的倍数向上取整倍数,ap就移动这么多
#define NEXT_SA(ap)	ap = (SA *) \
	((caddr_t) ap + (ap->sa_len ? ROUNDUP(ap->sa_len, sizeof (u_long)) : \
									sizeof(u_long)))

void
get_rtaddrs(int addrs, SA *sa, SA **rti_info)
{
	int		i;

	for (i = 0; i < RTAX_MAX; i++) {
		if (addrs & (1 << i)) { //看看8个掩码中哪一个被设置
			//刚传参进来的时候,sa指向rt_msghdr结构体后的第一个套接字地址结构
			//如果相应的位存在,则把sa指向的套接字地址结构放入指针数组rti_info中
			//并且用NEXT_SA这个宏指向下一个套接字地址结构
			//再继续循环
			rti_info[i] = sa; 
			NEXT_SA(sa); //sa指向下一个地址结构体的开始处
		} else
			rti_info[i] = NULL; //相应位不存在则置空
	}
}
