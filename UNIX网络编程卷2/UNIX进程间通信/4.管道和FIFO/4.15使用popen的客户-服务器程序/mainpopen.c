#include	"unpipc.h"

int
main(int argc, char **argv)
{
	size_t	n;
	char	buff[MAXLINE], command[MAXLINE];
	FILE	*fp;

		/* 4read pathname */
	Fgets(buff, MAXLINE, stdin); //从标准输入读入一行
	n = strlen(buff);		/* fgets() guarantees null byte at end 如果缓冲区buff是以'\n'结尾 */
	if (buff[n-1] == '\n') //那么不把这个'\n'用作命令,以此保证保存命令的缓冲区以空字符结尾
		n--;				/* delete newline from fgets() */

	snprintf(command, sizeof(command), "cat %s", buff); //格式化命令字符串,存放入command中
	fp = Popen(command, "r"); //以读的方式打开命令所代表的程序,此程序的标准输出将连接到管道

		/* 4copy from pipe to standard output */
	while (Fgets(buff, MAXLINE, fp) != NULL) //从fp中读
		Fputs(buff, stdout); //写到标准输出

	Pclose(fp);
	exit(0);
}
