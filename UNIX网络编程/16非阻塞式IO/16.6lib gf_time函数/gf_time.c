#include	"unp.h"
#include	<time.h>

char *
gf_time(void)
{
	struct timeval	tv;
	time_t			t;
	static char		str[30];
	char			*ptr;

	if (gettimeofday(&tv, NULL) < 0) //获得当前时间存放在tv中
		err_sys("gettimeofday error");

	//把tv结构体中的描述时间提取出来
	t = tv.tv_sec;	/* POSIX says tv.tv_sec is time_t; some BSDs don't agree. */
	ptr = ctime(&t); //把提取出来的秒数转换成表达式格式
	strcpy(str, &ptr[11]); //复制字符串,除去“Fri Sep 13”这11个字符,一共拷贝"00:00:00"这8个字符
		/* Fri Sep 13 00:00:00 1986\n\0 */
		/* 0123456789012345678901234 5  */
	//从前面已拷贝的8个字符后面继续开始拷贝,把微秒数拼接在后面
	snprintf(str+8, sizeof(str)-8, ".%06d", tv.tv_usec);

	return(str);
}
