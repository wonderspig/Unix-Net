#include	"unpipc.h"

#define	SEQFILE	"seqno"		/* filename */

void	my_lock(int), my_unlock(int);

int
main(int argc, char **argv)
{
	int		fd;
	long	i, seqno;
	pid_t	pid;
	ssize_t	n;
	char	line[MAXLINE + 1];

	pid = getpid(); //获得本进程的pid
	fd = Open(SEQFILE, O_RDWR, FILE_MODE); //打开文件,读写方式

	for (i = 0; i < 20; i++) { //循环20次
		my_lock(fd);				/* lock the file 锁住文件*/

		Lseek(fd, 0L, SEEK_SET);	/* rewind before read 把文件指针定位于文件开始处 */
		n = Read(fd, line, MAXLINE); //从这个文件中读取一行
		line[n] = '\0';				/* null terminate for sscanf 缓冲区空字符串结尾(sscanf需要以空字符结尾的字符串) */

		n = sscanf(line, "%ld\n", &seqno); //把从文件中读到的序列号以long变量的形式保存到seqno中
		printf("%s: pid = %ld, seq# = %ld\n", argv[0], (long) pid, seqno); //打印当前程序名,进程pid,文件中的序列号

		seqno++;					/* increment sequence number 把文件中读到的序列号+1*/

		snprintf(line, sizeof(line), "%ld\n", seqno); //把long型变量打印成表达式格式,存入line中
		Lseek(fd, 0L, SEEK_SET);	/* rewind before write 文件指针重新指向文件开头 */
		Write(fd, line, strlen(line)); //把line写回文件

		my_unlock(fd);				/* unlock the file 解锁文件 */
	}
	exit(0);
}
