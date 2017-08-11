#include	"unpipc.h"

int
main(int argc, char **argv)
{
	int		fd1, fd2, *ptr1, *ptr2;
	pid_t	childpid;
	struct stat	stat;

	if (argc != 2) //如果命令行参数不等于2,则出错
		err_quit("usage: test3 <name>");

	shm_unlink(Px_ipc_name(argv[1])); //先删除命令行指定的共享内存区对象
	fd1 = Shm_open(Px_ipc_name(argv[1]), O_RDWR | O_CREAT | O_EXCL, FILE_MODE); //打开命令行指定的共享内存区对象
	Ftruncate(fd1, sizeof(int)); //调整共享内存区对象大小为一个int的大小
	fd2 = Open("/etc/motd", O_RDONLY); //打开文件"/etc/motd"
	Fstat(fd2, &stat); //获得文件"/etc/motd"的属性
	//创建子进程
	if ( (childpid = Fork()) == 0) { //在子进程中
			/* 4child 和父进程相反顺序的映射两个共享内存区*/
		ptr2 = Mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd2, 0); 
		ptr1 = Mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
					MAP_SHARED, fd1, 0);
		printf("child: shm ptr = %p, motd ptr = %p\n", ptr1, ptr2); //打印两个共享内存区的起始地址

		sleep(5); //子进程睡5秒(等待父进程写入777)
		printf("shared memory integer = %d\n", *ptr1); //打印共享内存区中的数据(应该是777)
		exit(0);
	}
		/* 4parent: mmap in reverse order from child 和子进程相反顺序的映射两个共享内存区*/
	ptr1 = Mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
	ptr2 = Mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd2, 0);
	printf("parent: shm ptr = %p, motd ptr = %p\n", ptr1, ptr2); //打印两个共享内存区的起始地址
	*ptr1 = 777; //在子进程睡眠5秒的过程中,写入777
	Waitpid(childpid, NULL, 0); //等待子进程结束

	exit(0);
}
