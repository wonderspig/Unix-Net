#include	"unproute.h"
#include	<netinet/udp.h>
#include	<netinet/ip_var.h>
#include	<netinet/udp_var.h>		/* for UDPCTL_xxx constants */

int
main(int argc, char **argv)
{
	int		mib[4], val;
	size_t	len;

	mib[0] = CTL_NET; //关心网络模块
	mib[1] = AF_INET; //获得网际协议的变量
	mib[2] = IPPROTO_UDP; //关心UDP的协议
	mib[3] = UDPCTL_CHECKSUM; //关心UDP校验是否开启

	len = sizeof(val); 
	Sysctl(mib, 4, &val, &len, NULL, 0); //把mib填入函数获得结果
	printf("udp checksum flag: %d\n", val); //val为1或者0

	exit(0);
}
