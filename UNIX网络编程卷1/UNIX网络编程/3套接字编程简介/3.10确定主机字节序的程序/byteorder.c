#include	"unp.h"

int
main(int argc, char **argv)
{
	//设定一个联合
	union {
	  short  s; //存入一个整数
      char   c[sizeof(short)]; //分别访问这个整数的所有字节
    } un;

	un.s = 0x0102; //给联合中的整数赋值
	printf("%s: ", CPU_VENDOR_OS); //打印cpu的类型，厂家和操作系统版本
	if (sizeof(short) == 2) { //如果short类型占用2字节,进入此分支
		if (un.c[0] == 1 && un.c[1] == 2) //高序字节存储在起始地址
			printf("big-endian\n");
		else if (un.c[0] == 2 && un.c[1] == 1) //低序字节存储在起始地址
			printf("little-endian\n");
		else
			printf("unknown\n");
	} else
	//short不占用两个字节的情况下打印short占用多少字节
		printf("sizeof(short) = %lu\n", sizeof(short)); 

	exit(0);
}
