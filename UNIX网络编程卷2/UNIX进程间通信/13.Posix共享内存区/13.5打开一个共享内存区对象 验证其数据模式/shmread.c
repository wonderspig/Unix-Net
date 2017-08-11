#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		i, fd;
	struct stat	stat;
	unsigned char	c, *ptr;

	if (argc != 2) //如果命令行参数不等于1,则出错
		err_quit("usage: shmread <name>");

		/* 4open, get size, map */
	fd = Shm_open(argv[1], O_RDONLY, FILE_MODE); //以只读方式打开命令行参数指定的共享内存区对象
	Fstat(fd, &stat); //湖区这个共享内存区的属性
	ptr = Mmap(NULL, stat.st_size, PROT_READ,
			   MAP_SHARED, fd, 0); //mmap映射这个共享内存区
	Close(fd); //关闭该文件不影响已经映射的共享内存区

		/* 4check that ptr[0] = 0, ptr[1] = 1, etc. */
	for (i = 0; i < stat.st_size; i++) //读取并验证共享内存区的模式
		if ( (c = *ptr++) != (i % 256))
			err_ret("ptr[%d] = %d", i, c);

	exit(0);
}
