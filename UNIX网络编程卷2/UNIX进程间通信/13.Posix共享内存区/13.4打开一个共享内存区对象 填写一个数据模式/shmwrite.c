#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		i, fd;
	struct stat	stat;
	unsigned char	*ptr;

	if (argc != 2) //如果命令行参数不等于1,则出错
		err_quit("usage: shmwrite <name>");

		/* 4open, get size, map */
	fd = Shm_open(argv[1], O_RDWR, FILE_MODE); //按命令行参数指定的名称,读写模式打开一个对象
	Fstat(fd, &stat); //或者这个对象的属性
	ptr = Mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0); //mmap映射共享内存区
	Close(fd); //映射完成关闭文件不会影响到已经映射的内存

		/* 4set: ptr[0] = 0, ptr[1] = 1, etc. */
	for (i = 0; i < stat.st_size; i++) //把模式写入该共享内存区
		*ptr++ = i % 256;

	exit(0);
}
