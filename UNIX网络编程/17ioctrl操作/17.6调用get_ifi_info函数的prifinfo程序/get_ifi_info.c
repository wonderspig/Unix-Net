/* include get_ifi_info1 */
#include	"unpifi.h"

struct ifi_info *
get_ifi_info(int family, int doaliases)
{
	//需要返回的链表属性:ifi为节点,ifihead为头节点,指向下一个节点的指针
	struct ifi_info		*ifi, *ifihead, **ifipnext;
	//sockfd:ioctrl请求的套接字
	//lastlen:循环中最近的一次ifconf中的ifreq的大小(用来判断分配给ifreq结构体的大小是否足够)
	//flags:接口的标志
	//idx:AF_LINK地址族提供的索引,hlen:AF_LINK提供的硬件地址的长度
	int					sockfd, len, lastlen, flags, myflags, idx = 0, hlen = 0;
	//ptr,buf为指向ifconf中的ifreq结构体数组的缓冲区(联合使char*和ifreq结构体共享同一块内存)
	//lastname最近一次的去掉":"的名字存储(用来验证别名)
	//haddr用来存储硬件地址,sdlname为硬件地址所对应的名字
	char				*ptr, *buf, lastname[IFNAMSIZ], *cptr, *haddr, *sdlname;
	struct ifconf		ifc;
	//ifrcopy结构体是用来获取更多信息时使用的副本
	struct ifreq		*ifr, ifrcopy;
	struct sockaddr_in	*sinptr; //IP4地址
	struct sockaddr_in6	*sin6ptr; //IP6地址

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0); //创建套接字UDP,IP4

	//用固定长度的缓冲区来等待ioctl的填写,如果空间不足,做适当处理

	lastlen = 0; //记录当前填充到ifconf结构体中的ifreq的大小,初始化为0
	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess 初始化缓冲区大小 */
	//ioctl返回的结果比缓冲区大时,ioctl可能不返回错误,而是直接截断,并返回成功
	//所以采取发出请求,记录下长度,然后用更大的长度去发出请求,两个长度相等,才说明我们的缓冲区足够大
	for ( ; ; ) {
		buf = Malloc(len); //动态分配ifconf的大小,包含大小为100个ifreq的大小
		ifc.ifc_len = len; //指明ifconf所包含的ifreq结构体数组的大小
		ifc.ifc_buf = buf; //ifreq结构体的缓冲区(联合使buf和结构体ifreq共享同一块内存)
		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) { //用ioctrl获得所有接口的列表,填写在ifc中
			if (errno != EINVAL || lastlen != 0) //在缓冲区太小时,会返回EINVAL错误
				err_sys("ioctl error");
		} else { //处理缓冲区太小而不反悔错误的情况
			if (ifc.ifc_len == lastlen)
				break;		/* success, len has not changed 填写成功,且大小和上一次循环没有改变 */
			lastlen = ifc.ifc_len; //记录当前填充到ifconf结构体中的ifreq的大小
		}
		len += 10 * sizeof(struct ifreq);	/* increment 用更大的缓冲区,再发出一次请求 */
		free(buf); 
	}

	//初始化链表的相关成员(构建链表结构)

	ifihead = NULL; //链表头节点指针为null
	ifipnext = &ifihead; //链表的"下一个节点指针"的指针,指向头节点指针的地址(到时候解引用就能为"头节点"或者"&ifi->ifi_next"附值)
	lastname[0] = 0; //lastname最近一次的去掉":"的名字存储(用来验证别名)
	sdlname = NULL; //硬件地址所对应的名称
/* end get_ifi_info1 */

/* include get_ifi_info2 */
	//buf指向ifconf中的第一个ifreq结构体数组的第一个成员的开始位置处,ptr不能超过buf+ifc.ifc_len
	//(以此来循环)
	//buf,ptr是指针,指向char*,所以+1代表的是指针右移一个字节单位
	//ifc.ifc_len是整个ifreq结构体数组的总的字节数,所以buf+ifc.ifc_len代表整个ifreq结构体数组的最后一个成员的末尾位置
	//(所以ptr自然不能超过buf+ifc.ifc_len)
	for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
		//ifr指向ifreq结构体数组中的每一个成员
		ifr = (struct ifreq *) ptr; //ptr是char*,也是是caddr_t,自然能够强转成联合的另一个成员ifreq

//计算出ifconf中的ifreq中的union ifr_ifru的地址大小"len"

#ifdef	HAVE_SOCKADDR_SA_LEN //如果定义了HAVE_SOCKADDR_SA_LEN这个宏(提供套接字地址结构的较新系统)
		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len); //长度取"通用地址长度"和"ifconf中的ifreq中的地址长度",取较大值
#else //不提供套接字地址结构的较老系统
		switch (ifr->ifr_addr.sa_family) { //按不同的地址族进行情况分析
#ifdef	IPV6 //如果主机支持IP6
		case AF_INET6: //IP6地址族
			len = sizeof(struct sockaddr_in6); //长度取IP6的地址结构体大小
			break;
#endif
		case AF_INET: //IP4地址族
		default:	
			len = sizeof(struct sockaddr); //地址取通用地址结构体大小
			break;
		}

//fconf中的ifreq中的union ifr_ifru的大小"len"+ifr->ifr_name的大小,就是整个ifreq结构结构的大小(单位是字节)
//ptr是char*指针,移动一个单位是一个字节的距离

#endif	/* HAVE_SOCKADDR_SA_LEN */
		ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer 
											   ptr指向下一个缓冲区(ifreq结构)
											   一个ifreq结构结构的大小是ifr_name+union ifr_ifru的大小
											   联合union ifr_ifru的大小以由条件编译中的len计算出来了*/

//如果系统支持,取出硬件地址和索引(等待后面填入链表节点)

#ifdef	HAVE_SOCKADDR_DL_STRUCT //如果定义了HAVE_SOCKADDR_DL_STRUCT,系统支持返回AF_LINK地址族的sockaddr_dl结构
								//AF_LINK这个地址族支持索引和硬件地址
		/* assumes that AF_LINK precedes AF_INET or AF_INET6 */
		if (ifr->ifr_addr.sa_family == AF_LINK) { //地址族是AF_LINK
			//把ifconf中的ifreq中的地址强转成(sockaddr_dl *)
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)&ifr->ifr_addr;
			sdlname = ifr->ifr_name; //把ifconf中的ifreq中的名字存放到sdlname
			idx = sdl->sdl_index; //取出AF_LINK地址族地址的索引
			haddr = sdl->sdl_data + sdl->sdl_nlen; //取出硬件地址
			hlen = sdl->sdl_alen; //获得硬件地址的长度
		}
#endif

		if (ifr->ifr_addr.sa_family != family)
			continue;	/* ignore if not desired address family 忽略所有不期望的地址族(调用者期望,传参进来的参数) */

//判断接口是否为别名

		myflags = 0; //标志
		if ( (cptr = strchr(ifr->ifr_name, ':')) != NULL) //查找":"在字符串ifr->ifr_name,中首次出现的位置
			*cptr = 0;		/* replace colon with null 删除字符串中的这个":",替换成null */
		//因为把":"替换成null,所以这里的strncmp只比较冒号前面的部分
		if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
			if (doaliases == 0) //如果传参进来的doaliases为空,则不处理别名,直接重新循环
				continue;	/* already processed this interface 当前的接口为前一个接口的别名,忽略当前接口*/
			myflags = IFI_ALIAS; //我们的自定义标志定义为别名宏
		}
		memcpy(lastname, ifr->ifr_name, IFNAMSIZ); //深拷贝ifconf中的ifreq中的名字到lastname(为了下一次循环验证是否为别名)

//只取出处于工作状态的接口(根据标志判断)

		ifrcopy = *ifr; //把ifconf中的ifreq结构体数组的指针,深拷贝一份
		Ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy); //获取接口标志,填入ifrcopy
		flags = ifrcopy.ifr_flags; //取出接口标志
		if ((flags & IFF_UP) == 0) //判断接口标志中是否有IFF_UP(IFF_UP表示是否处于工作状态)
			continue;	/* ignore if interface not up 忽略所有接口标志不是IFF_UP的接口 */
/* end get_ifi_info2 */

/* include get_ifi_info3 */

//把当前指针放在放在链表的末尾

		ifi = Calloc(1, sizeof(struct ifi_info)); //分配一个大小为一个ifi_info结构体的内存空间,每一位都置0
		//ifipnext是二级指针,是指向"一个节点的指针"的指针,所以*ifipnext就是"一个节点的指针"
		//第一轮循环:*ifipnext是ifihead(头节点)
		//第二轮循环:*ifipnext是&ifi->ifi_next
		//把当前指针放在放在链表的末尾
		*ifipnext = ifi;			/* prev points to this new one */
		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

//取出标志和自定义标志

		ifi->ifi_flags = flags;		/* IFF_xxx values 把获得的标志放入链表节点的标志成员 */
		ifi->ifi_myflags = myflags;	/* IFI_xxx values 把设置好的自定义标志成员放入链表的对应成员 */

//获得MTU值

#if defined(SIOCGIFMTU) && defined(HAVE_STRUCT_IFREQ_IFR_MTU) //如果定义了SIOCGIFMTU和HAVE_STRUCT_IFREQ_IFR_MTU
		Ioctl(sockfd, SIOCGIFMTU, &ifrcopy); //获得接口的MTU值,填入ifrcopy
		ifi->ifi_mtu = ifrcopy.ifr_mtu; //把MTU的值放入链表节点的相关成员中
#else
		ifi->ifi_mtu = 0; //如系统不支持获取MTU值,则附值为0
#endif

//填写接口名字

		memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME); //把ifconf中的freq结构体中的名字填入链表节点的相关成员中
		ifi->ifi_name[IFI_NAME-1] = '\0'; //格式化成null结尾

//根据情况判断是否提取索引和硬件地址填入链表节点

		/* If the sockaddr_dl is from a different interface, ignore it
		   判断sockaddr_dl结构的实现是否如预期,不如预期就忽略它 */
		if (sdlname == NULL || strcmp(sdlname, ifr->ifr_name) != 0)
			idx = hlen = 0; //把索引和硬件地址长度都置为0
		//如和预期一样,则把硬件地址长度和索引填入链表节点中
		ifi->ifi_index = idx; 
		ifi->ifi_hlen = hlen;
		if (ifi->ifi_hlen > IFI_HADDR) //如果硬件地址长度太大,则设置成最大值
			ifi->ifi_hlen = IFI_HADDR;
		if (hlen) //此判断代表硬件地址确实正常获取,拷贝内存地址存放的内存块到链表节点中
			memcpy(ifi->ifi_haddr, haddr, ifi->ifi_hlen);
/* end get_ifi_info3 */
/* include get_ifi_info4 */

//根据地址族填写ip4的地址,广播地址,点对点链路地址
//IP6地址族的地址,和点对点链路地址(ip6不支持广播地址)

		switch (ifr->ifr_addr.sa_family) { //根据地址族填写地址
		case AF_INET: //IP4地址族
			sinptr = (struct sockaddr_in *) &ifr->ifr_addr; //把ifconf中的freq结构体中的地址强转成IP4的地址格式
			ifi->ifi_addr = Calloc(1, sizeof(struct sockaddr_in)); //动态分配存放地址的空间,初始化为0
			memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in)); //把地址拷贝进去

#ifdef	SIOCGIFBRDADDR //如果支持广播
			if (flags & IFF_BROADCAST) { //接口标志带有广播标志
				Ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy); //获得广播地址填入ifrcopy结构体
				sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr; //强转广播地址为IP4地址
				ifi->ifi_brdaddr = Calloc(1, sizeof(struct sockaddr_in)); //为广播地址分配存储空间,初始化为0
				memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in)); //把广播地址拷贝进去
			}
#endif

#ifdef	SIOCGIFDSTADDR //支持点到点链路地址
			if (flags & IFF_POINTOPOINT) { //带有点到点链路地址的标志
				Ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy); //获得点到点链路地址,填入ifrcopy
				sinptr = (struct sockaddr_in *) &ifrcopy.ifr_dstaddr; //强转点到点链路地址为IP4
				ifi->ifi_dstaddr = Calloc(1, sizeof(struct sockaddr_in)); //分配点到点链路地址存储空间,初始化为0
				memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in)); //把点到点链路地址拷贝进去
			}
#endif
			break;

		case AF_INET6: //IP6地址族
			sin6ptr = (struct sockaddr_in6 *) &ifr->ifr_addr; //把ifconf中的freq结构体中的地址强转成IP6的地址格式
			ifi->ifi_addr = Calloc(1, sizeof(struct sockaddr_in6)); //动态分配存放地址的空间,初始化为0
			memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6)); //把地址拷贝进去

#ifdef	SIOCGIFDSTADDR //支持点到点链路地址
			if (flags & IFF_POINTOPOINT) { //带有点到点链路地址的标志
				Ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy); //获得点到点链路地址,填入ifrcopy
				sin6ptr = (struct sockaddr_in6 *) &ifrcopy.ifr_dstaddr; //强转点到点链路地址为IP6
				ifi->ifi_dstaddr = Calloc(1, sizeof(struct sockaddr_in6)); //分配点到点链路地址存储空间,初始化为0
				memcpy(ifi->ifi_dstaddr, sin6ptr, sizeof(struct sockaddr_in6)); //把点到点链路地址拷贝进去
			}
#endif
			break;

		default:
			break;
		}
	}
	free(buf); //buf指针指向的是ifconf中的freq结构体(通过联合共享同一块内存),已经把其中的数据全部放入了链表中,可以释放了
	return(ifihead);	/* pointer to first structure in linked list */
}
/* end get_ifi_info4 */

/* include free_ifi_info */
void
free_ifi_info(struct ifi_info *ifihead)
{
	struct ifi_info	*ifi, *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext) { //从头节点开始遍历链表
		//在每个链表节点中,有三个成员也是动态分配内存的,也需要释放
		if (ifi->ifi_addr != NULL) 
			free(ifi->ifi_addr);
		if (ifi->ifi_brdaddr != NULL)
			free(ifi->ifi_brdaddr);
		if (ifi->ifi_dstaddr != NULL)
			free(ifi->ifi_dstaddr);
		ifinext = ifi->ifi_next;	/* can't fetch ifi_next after free() 
									   ifi释放后,无法再取得ifi->ifi_next */
		free(ifi);					/* the ifi_info{} itself */
	}
}
/* end free_ifi_info */

struct ifi_info *
Get_ifi_info(int family, int doaliases)
{
	struct ifi_info	*ifi;

	if ( (ifi = get_ifi_info(family, doaliases)) == NULL)
		err_quit("get_ifi_info error");
	return(ifi);
}
