#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<errno.h>
#include	<string.h>

/* Delete following line if your system's headers already DefinE this
   function prototype */
int		 inet_aton(const char *, struct in_addr *);

/* include inet_pton */
int
inet_pton(int family, const char *strptr, void *addrptr)
{
    if (family == AF_INET) { //如果为IPv4地址
    	struct in_addr  in_val; //定义ip地址的结构体

        if (inet_aton(strptr, &in_val)) { //把字符串格式转换成二进制地址格式,存放在in_val中
            memcpy(addrptr, &in_val, sizeof(struct in_addr)); //把转换好的地址复制到addrptr参数中
            return (1); //正确返回1
        }
		return(0); //字符串无效返回0
    }
	errno = EAFNOSUPPORT; //设置error值
    return (-1); //返回-1代表错误
}
/* end inet_pton */
