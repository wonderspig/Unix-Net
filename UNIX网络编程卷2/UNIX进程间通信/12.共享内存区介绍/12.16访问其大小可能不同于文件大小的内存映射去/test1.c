#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		fd, i;
	char	*ptr;
	size_t	filesize, mmapsize, pagesize;

	if (argc != 4) //命令行参数不等于3,则出错
		err_quit("usage: test1 <pathname> <filesize> <mmapsize>");
	filesize = atoi(argv[2]); //第二个参数为文件大小
	mmapsize = atoi(argv[3]); //第三个参数为映射区大小

		/* 4open file: create or truncate; set file size */
	fd = Open(argv[1], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE); //打开或创建第一个命令行参数所指定的文件
	Lseek(fd, filesize-1, SEEK_SET); //文件指针移动到预定文件大小-1的位置
	Write(fd, "", 1); //往文件中写一个字节,此时文件指针移动到预定文件大小的位置

	ptr = Mmap(NULL, mmapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //按照命令行第三个参数所指定的待下来映射文件
	Close(fd); //映射完了关闭文件,并不影响已映射的文件

	pagesize = Sysconf(_SC_PAGESIZE); //获得系统实现的页面大小
	printf("PAGESIZE = %ld\n", (long) pagesize); //打印系统实现的页面大小

	for (i = 0; i < max(filesize, mmapsize); i += pagesize) { //打印内存映射区的每个页面的首字节和尾字节
		printf("ptr[%d] = %d\n", i, ptr[i]); //打印首字节
		ptr[i] = 1; //把首字节置为1
		printf("ptr[%d] = %d\n", i + pagesize - 1, ptr[i + pagesize - 1]); //打印尾字节
		ptr[i + pagesize - 1] = 1; //把尾字节置为1
	}
	//此时的i要么大于等于filesize,要么大于等于mmapsize
	printf("ptr[%d] = %d\n", i, ptr[i]); //循环结束后,输出下一个页面的首字节
	
	exit(0);
}
