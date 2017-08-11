#include	"unpifi.h"
#include	"unproute.h"

/* include get_ifi_info1 */
struct ifi_info *
get_ifi_info(int family, int doaliases)
{
	int 				flags;
	char				*buf, *next, *lim;
	size_t				len;
	struct if_msghdr	*ifm;
	struct ifa_msghdr	*ifam;
	struct sockaddr		*sa, *rti_info[RTAX_MAX];
	struct sockaddr_dl	*sdl;
	struct ifi_info		*ifi, *ifisave, *ifihead, **ifipnext;

	buf = Net_rt_iflist(family, 0, &len);

	//链表结构初始化
	ifihead = NULL; //头节点为null
	ifipnext = &ifihead; //后节点初始化为头节点

	lim = buf + len; //lim指向buf缓冲区的末尾的最后一个字节(虽然buf没有指定大小,但是循环不能超过这个位置)
	for (next = buf; next < lim; next += ifm->ifm_msglen) { //所有路由消息的前三个成员是相同的(我们只是为了查看ifm_type成员)
		ifm = (struct if_msghdr *) next; //先把next强转成if_msghdr(假设是if_msghdr,只是为了查看ifm_type成员)
		//sysctl为每个借口返回一个RTM_IFINFO消息
		if (ifm->ifm_type == RTM_IFINFO) { //判断消息类型(RTM_IFINFO消息代表接口开工或停工)
			if ( ((flags = ifm->ifm_flags) & IFF_UP) == 0) //如果接口没有开工(停工)
				continue;	/* ignore if interface not up 忽略这个接口,继续循环 */

			//代码走到这里,说明接口是开工的
			sa = (struct sockaddr *) (ifm + 1); //指针跳过消息头部,指向第一个地址结构体
			get_rtaddrs(ifm->ifm_addrs, sa, rti_info); //根据掩码,填写rti_info地址指针数组
			if ( (sa = rti_info[RTAX_IFP]) != NULL) { //如果有携带接口名字的套接字地址结构
				ifi = Calloc(1, sizeof(struct ifi_info)); //动态分配一个初始化为0的ifi_info节点
				//把这个节点添加再链表末尾
				*ifipnext = ifi;			/* prev points to this new one *ifipnext指向头节点指针,也就是在为头节点指针附值*/
				//给头节点附值完成后,*ifipnext指向"指向下一个节点的指针"的指针
				ifipnext = &ifi->ifi_next;	/* ptr to next one goes here */

				ifi->ifi_flags = flags; //把标志填入节点
				if (sa->sa_family == AF_LINK) { //如果是连路层地址
					sdl = (struct sockaddr_dl *) sa; //强转成连路层地址结构体
					ifi->ifi_index = sdl->sdl_index; //把索引填入链表节点
					if (sdl->sdl_nlen > 0) //名字的长度不为0
						snprintf(ifi->ifi_name, IFI_NAME, "%*s",
								 sdl->sdl_nlen, &sdl->sdl_data[0]); //把名字填入结构体
					else
						snprintf(ifi->ifi_name, IFI_NAME, "index %d",
								 sdl->sdl_index); //否则填入索引号

					if ( (ifi->ifi_hlen = sdl->sdl_alen) > 0) //如果硬件地址长度不为0
						memcpy(ifi->ifi_haddr, LLADDR(sdl),
							   min(IFI_HADDR, sdl->sdl_alen)); //把硬件地址填写入结构体
				}
			}
/* end get_ifi_info1 */

/* include get_ifi_info3 */
		} else if (ifm->ifm_type == RTM_NEWADDR) { //消息类型是RTM_NEWADDR(此消息包括主地址和所有别名地址)
			if (ifi->ifi_addr) {	/* already have an IP addr for i/f 代表有别名地址 */
				if (doaliases == 0) //参数表示调用者不需要别名地址
					continue;

					/* 4we have a new IP addr for existing interface */
				ifisave = ifi; //把ifi浅复制一份
				ifi = Calloc(1, sizeof(struct ifi_info)); //为ifi分配空间,初始化为0
				//放入链表数据结构的末尾
				*ifipnext = ifi;			/* prev points to this new one */
				ifipnext = &ifi->ifi_next;	/* ptr to next one goes here */

				//标志,索引,接口名长度,接口名,硬件地址,都是和非别名的时候一样
				ifi->ifi_flags = ifisave->ifi_flags; //填写入标志
				ifi->ifi_index = ifisave->ifi_index; //填写入索引
				ifi->ifi_hlen = ifisave->ifi_hlen; //填写接口名长度
				memcpy(ifi->ifi_name, ifisave->ifi_name, IFI_NAME);  //填写接口名
				memcpy(ifi->ifi_haddr, ifisave->ifi_haddr, IFI_HADDR); //填写硬件地址
			}

			ifam = (struct ifa_msghdr *) next; //强转next为ifa_msghdr结构体
			sa = (struct sockaddr *) (ifam + 1); //sa指向第一个套接字地址结构体的开始处
			get_rtaddrs(ifam->ifam_addrs, sa, rti_info); //根据掩码把套接字地址结构填写在rti_info地址结构指针数组中

			if ( (sa = rti_info[RTAX_IFA]) != NULL) { //携带接口名的地址结构
				ifi->ifi_addr = Calloc(1, sa->sa_len);
				memcpy(ifi->ifi_addr, sa, sa->sa_len);
			}

			if ((flags & IFF_BROADCAST) &&
				(sa = rti_info[RTAX_BRD]) != NULL) { //广播地址结构
				ifi->ifi_brdaddr = Calloc(1, sa->sa_len);
				memcpy(ifi->ifi_brdaddr, sa, sa->sa_len);
			}

			if ((flags & IFF_POINTOPOINT) &&
				(sa = rti_info[RTAX_BRD]) != NULL) { //点对点链路地址结构
				ifi->ifi_dstaddr = Calloc(1, sa->sa_len);
				memcpy(ifi->ifi_dstaddr, sa, sa->sa_len);
			}

		} else
			err_quit("unexpected message type %d", ifm->ifm_type);
	}
	/* "ifihead" points to the first structure in the linked list */
	return(ifihead);	/* ptr to first structure in linked list 返回头节点 */
}
/* end get_ifi_info3 */

void
free_ifi_info(struct ifi_info *ifihead)
{
	struct ifi_info	*ifi, *ifinext;

	for (ifi = ifihead; ifi != NULL; ifi = ifinext) {
		if (ifi->ifi_addr != NULL)
			free(ifi->ifi_addr);
		if (ifi->ifi_brdaddr != NULL)
			free(ifi->ifi_brdaddr);
		if (ifi->ifi_dstaddr != NULL)
			free(ifi->ifi_dstaddr);
		ifinext = ifi->ifi_next;		/* can't fetch ifi_next after free() */
		free(ifi);					/* the ifi_info{} itself */
	}
}

struct ifi_info *
Get_ifi_info(int family, int doaliases)
{
	struct ifi_info	*ifi;

	if ( (ifi = get_ifi_info(family, doaliases)) == NULL)
		err_quit("get_ifi_info error");
	return(ifi);
}
