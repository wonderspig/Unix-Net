#include	"unpipc.h"

#define	FILE	"test.data"
#define	SIZE	32768

int
main(int argc, char **argv)
{
	int		fd, i;
	char	*ptr;

		/* 4open: create or truncate; then mmap file */
	fd = Open(FILE, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE); //创建截断为0,一个文件
	ptr = Mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //映射这个文件,映射大小为SIZE

	for (i = 4096; i <= SIZE; i += 4096) { 
		printf("setting file size to %d\n", i);
		Ftruncate(fd, i); //该文件大小每次增长4096个字节
		printf("ptr[%d] = %d\n", i-1, ptr[i-1]);
	}
	
	exit(0);
}
