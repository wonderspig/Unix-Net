/* include if_nameindex */
#include	"unpifi.h"
#include	"unproute.h"

struct if_nameindex *
if_nameindex(void)
{
	char				*buf, *next, *lim;
	size_t				len;
	struct if_msghdr	*ifm;
	struct sockaddr		*sa, *rti_info[RTAX_MAX];
	struct sockaddr_dl	*sdl;
	struct if_nameindex	*result, *ifptr;
	char				*namptr;

	if ( (buf = net_rt_iflist(0, 0, &len)) == NULL) //返回所有接口(任何地址族,任何索引号)
		return(NULL);

	if ( (result = malloc(len)) == NULL)	/* overestimate 分配存放结果的地址空间(用来存放返回给调用者的if_nameindex数组)*/
		return(NULL);
	ifptr = result; //把存放结果地址空间的指针浅复制一份(指向数组开始处)
	namptr = (char *) result + len;	/* names start at end of buffer 指向数组结尾*/

	lim = buf + len; //lim指向buf缓冲区的末尾
	for (next = buf; next < lim; next += ifm->ifm_msglen) { //用循环遍历每一个消息(整个buf缓冲区中的消息)
		ifm = (struct if_msghdr *) next; //把next强转成if_msghdr,为了访问ifm_type字段
		if (ifm->ifm_type == RTM_IFINFO) { //判断ifm_type字段是否为RTM_IFINFO
			sa = (struct sockaddr *) (ifm + 1); //sa指向消息后的第一个结构体地址
			get_rtaddrs(ifm->ifm_addrs, sa, rti_info); //把每一个结构体地址的指针存放入rti_info指针数组中
			if ( (sa = rti_info[RTAX_IFP]) != NULL) { //判断接口地址是否存在
				if (sa->sa_family == AF_LINK) { //如果为AF_LINK地址族
					sdl = (struct sockaddr_dl *) sa; //强转sa为sockaddr_dl结构体
					//namptr现在指向缓冲区的末尾(从缓冲区的后面往前面填写)

					//从缓冲区末尾开始存放接口名字,从缓冲区正方向构建if_nameindex数组

					namptr -= sdl->sdl_nlen + 1; //使namptr指向的位置填入一个地址后,正好处于缓冲区末尾倒数一个字节
					strncpy(namptr, &sdl->sdl_data[0], sdl->sdl_nlen); //填入地址
					namptr[sdl->sdl_nlen] = 0;	/* null terminate 空字符结尾*/
					ifptr->if_name = namptr; //把指针填入数组
					ifptr->if_index = sdl->sdl_index; //把索引填入数组
					ifptr++; //指向数组的第二个元素
				}
			}

		}
	}
	ifptr->if_name = NULL;	/* mark end of array of structs 数组的最后一个元素为null*/
	ifptr->if_index = 0;
	free(buf);
	return(result);			/* caller must free() this when done 返回数组首地址*/
}
/* end if_nameindex */

/* include if_freenameindex */
void
if_freenameindex(struct if_nameindex *ptr) //释放数组所在的内存空间
{
	free(ptr);
}
/* end if_freenameindex */

struct if_nameindex *
If_nameindex(void)
{
	struct if_nameindex	*ifptr;

	if ( (ifptr = if_nameindex()) == NULL)
		err_quit("if_nameindex error");
	return(ifptr);
}
