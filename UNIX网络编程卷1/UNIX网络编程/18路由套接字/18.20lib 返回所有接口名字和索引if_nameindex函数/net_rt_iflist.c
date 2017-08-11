/* include net_rt_iflist */
#include	"unproute.h"

char *
net_rt_iflist(int family, int flags, size_t *lenp)
{
	int		mib[6];
	char	*buf;

	mib[0] = CTL_NET; //关心网络子系统
	mib[1] = AF_ROUTE; //关心路由
	mib[2] = 0; //AF_ROUTE后只能为0
	mib[3] = family;		/* only addresses of this family 把传参进来的地址族填入 */
	mib[4] = NET_RT_IFLIST; //关心接口列表
	mib[5] = flags;			/* interface index or 0 接口索引 */
	if (sysctl(mib, 6, NULL, lenp, NULL, 0) < 0) //通过填写好的mib数组确定返回的大小:lenp的值
		return(NULL);

	if ( (buf = malloc(*lenp)) == NULL) //根据lenp的值动态分配buf的空间
		return(NULL);
	if (sysctl(mib, 6, buf, lenp, NULL, 0) < 0) { //调用sysctl把信息填写入buf
		free(buf);
		return(NULL);
	}

	return(buf);
}
/* end net_rt_iflist */

char *
Net_rt_iflist(int family, int flags, size_t *lenp)
{
	char	*ptr;

	if ( (ptr = net_rt_iflist(family, flags, lenp)) == NULL)
		err_sys("net_rt_iflist error");
	return(ptr);
}
