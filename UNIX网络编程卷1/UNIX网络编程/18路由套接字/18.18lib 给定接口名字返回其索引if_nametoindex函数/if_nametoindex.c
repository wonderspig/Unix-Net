/* include if_nametoindex */
#include	"unpifi.h"
#include	"unproute.h"

unsigned int
if_nametoindex(const char *name)
{
	unsigned int		idx, namelen;
	char				*buf, *next, *lim;
	size_t				len;
	struct if_msghdr	*ifm;
	struct sockaddr		*sa, *rti_info[RTAX_MAX];
	struct sockaddr_dl	*sdl;

	if ( (buf = net_rt_iflist(0, 0, &len)) == NULL) //获取任何地址族,任何索引的接口列表
		return(0);

	namelen = strlen(name); //计算一下name缓冲区的大小
	lim = buf + len; //len为消息的总长度,buf指针的单位是1个字节,所以lim指向buf的末尾字节
	//sysctl为每一个接口返回一个RTM_IFINFO类型的消息
	for (next = buf; next < lim; next += ifm->ifm_msglen) {
		ifm = (struct if_msghdr *) next; //把next强转成if_msghdr,用来查看ifm_type字段(任何结构都有ifm_type字段)
		if (ifm->ifm_type == RTM_IFINFO) { //如果消息类型是RTM_IFINFO,此消息使用if_msghdr结构
			sa = (struct sockaddr *) (ifm + 1); //sa指向第一个地址结构体的开始处
			get_rtaddrs(ifm->ifm_addrs, sa, rti_info); //把每一个地址结构体的指针放入rti_info指针数组
			if ( (sa = rti_info[RTAX_IFP]) != NULL) { //下标为RTAX_IFP的是接口地址
				if (sa->sa_family == AF_LINK) { //如果地址族是AF_LINK
					sdl = (struct sockaddr_dl *) sa; //sa强转成链路地址
					//验证传参进来的地址和循环到的这个地址是否相等
					if (sdl->sdl_nlen == namelen && strncmp(&sdl->sdl_data[0], name, sdl->sdl_nlen) == 0) {
						idx = sdl->sdl_index;	/* save before free() 相等则把索引保存出来*/
						free(buf);
						return(idx); //返回索引
					}
				}
			}

		}
	}
	free(buf); 
	return(0);		/* no match for name 不相等则返回null */
}
/* end if_nametoindex */

unsigned int
If_nametoindex(const char *name)
{
	int		idx;

	if ( (idx = if_nametoindex(name)) == 0)
		err_quit("if_nametoindex error for %s", name);
	return(idx);
}
