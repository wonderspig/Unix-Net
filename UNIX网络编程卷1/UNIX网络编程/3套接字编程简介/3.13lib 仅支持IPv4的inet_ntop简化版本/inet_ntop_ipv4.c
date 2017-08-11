#include	<sys/types.h>
#include	<sys/socket.h>
#include	<errno.h>
#include	<stdio.h>
#include    <string.h>

#ifndef	INET_ADDRSTRLEN
#define	INET_ADDRSTRLEN		16  //定义ip4地址的长度
#endif

/* include inet_ntop */
const char *
inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const u_char *p = (const u_char *) addrptr; //把传参进来的二进制地址强转成u_char类型

	if (family == AF_INET) { //如果为ip4地址
		char	temp[INET_ADDRSTRLEN]; //定义一个足够存放ip地址大小的数组

		//把传参进来的已经转换成u_char的二进制地址分解成x.x.x.x的形式并存放入temp这个数组中
		snprintf(temp, sizeof(temp), "%d.%d.%d.%d",
				 p[0], p[1], p[2], p[3]); 
		if (strlen(temp) >= len) { //把temp这个数组的大小和传参进来的调用者用来存放的数组大小进行比较
			errno = ENOSPC; //temp的大小大，调用者的缓存大小不够，则出错，设置error值
			return (NULL); //并且返回NULL
		}
		strcpy(strptr, temp); //否则把temp复制到调用者传参的缓存中
		return (strptr); //并且返回这个缓存的指针
	}
	errno = EAFNOSUPPORT; //不是ip4地址
	return (NULL); //返回NULL
}
/* end inet_ntop */
