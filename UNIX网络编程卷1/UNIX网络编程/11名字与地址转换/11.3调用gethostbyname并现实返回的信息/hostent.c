#include	"unp.h"

int
main(int argc, char **argv)
{
	char			*ptr, **pptr;
	char			str[INET_ADDRSTRLEN];
	struct hostent	*hptr;

	while (--argc > 0) { //有命令行参数传入
		ptr = *++argv; //提取第一个命令行参数
		if ( (hptr = gethostbyname(ptr)) == NULL) { //用主机名来获得hptr结构体
			err_msg("gethostbyname error for host: %s: %s",
					ptr, hstrerror(h_errno)); //出错则打印错误消息
			continue;
		}
		//代码走到这里说明获取hptr结构成功
		printf("official hostname: %s\n", hptr->h_name); //打印标准主机名

		for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) 
			printf("\talias: %s\n", *pptr); //用循环打印主机的所有别名

		switch (hptr->h_addrtype) { //判断ip地址的类型
		case AF_INET: //如果是ip4地址
			pptr = hptr->h_addr_list; 
			for ( ; *pptr != NULL; pptr++) //用循环遍历链表
				//把二进制地址格式转换成字符串格式并打印出来
				printf("\taddress: %s\n",
					Inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str))); 
			break;

		default: //不是ip4地址则出错
			err_ret("unknown address type");
			break;
		}
	}
	exit(0);
}
