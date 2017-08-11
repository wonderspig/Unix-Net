#include	"unpipc.h"

void
client(int readfd, int writefd)
{
	size_t	len;
	ssize_t	n;
	char	buff[MAXLINE];

		/* 4read pathname */
	Fgets(buff, MAXLINE, stdin); //从标准输入读入一行
	len = strlen(buff);		/* fgets() guarantees null byte at end 计算一下从标准输入读入了多少个字节 */
	if (buff[len-1] == '\n') //buff缓冲区的末尾如果是'\n'
		len--;				/* delete newline from fgets() 删除这个'\n',换行符不输出给服务器(子进程)*/

		/* 4write pathname to IPC channel */
	Write(writefd, buff, len); //把数据通过管道传送给服务器(子进程)

		/* 4read from IPC, write to standard output */
	while ( (n = Read(readfd, buff, MAXLINE)) > 0) //循环从管道中读取消息,直到读到小于等于0的结果为止(意味着读完了)
		Write(STDOUT_FILENO, buff, n); //把读到的消息写到标准输出
}
