/* include if_indextoname */
#include	"unpifi.h"
#include	"unproute.h"

char *
if_indextoname(unsigned int idx, char *name)
{
	char				*buf, *next, *lim;
	size_t				len;
	struct if_msghdr	*ifm;
	struct sockaddr		*sa, *rti_info[RTAX_MAX];
	struct sockaddr_dl	*sdl;

	if ( (buf = net_rt_iflist(0, idx, &len)) == NULL) //把所引号所对应的地址填写在buf中
		return(NULL);

	lim = buf + len; //计算出buf末尾的位置
	for (next = buf; next < lim; next += ifm->ifm_msglen) { //用循环遍历每一个消息
		ifm = (struct if_msghdr *) next; //把next强转成if_msghdr(RTM_IFINFO消息)
		if (ifm->ifm_type == RTM_IFINFO) { //判断消息类型
			sa = (struct sockaddr *) (ifm + 1); //sa指向第一个地址结构体
			get_rtaddrs(ifm->ifm_addrs, sa, rti_info); //把每个地址结构体指针存入rti_info指针数组
			if ( (sa = rti_info[RTAX_IFP]) != NULL) { //判断是否有结构地址
				if (sa->sa_family == AF_LINK) { //接口地址的地址族是否为AF_LINK
					sdl = (struct sockaddr_dl *) sa; //强转sa链路层地址
					if (sdl->sdl_index == idx) { //判断索引号是否和传参进来的相同
						int slen = min(IFNAMSIZ - 1, sdl->sdl_nlen); //计算名称字符串的长度
						strncpy(name, sdl->sdl_data, slen); //拷贝名称字符串到name中
						name[slen] = 0;	/* null terminate 空自负结尾*/
						free(buf);
						return(name); //返回名字字符串
					} 
				}
			}

		}
	}
	free(buf);
	return(NULL);		/* no match for index 没找到相同的索引号则返回空*/
}
/* end if_indextoname */

char *
If_indextoname(unsigned int idx, char *name)
{
	char	*ptr;

	if ( (ptr = if_indextoname(idx, name)) == NULL)
		err_quit("if_indextoname error for %d", idx);
	return(ptr);
}
