#include	"web.h"

void
write_get_cmd(struct file *fptr)
{
	int		n;
	char	line[MAXLINE];
	//宏GET_CMD中定义了字符串的表达式格式,其中的%s由文件结构体中的名字成员填充
	n = snprintf(line, sizeof(line), GET_CMD, fptr->f_name); //格式化需要发送给服务器的命令到line
	Writen(fptr->f_fd, line, n); //把line发送到服务器
	printf("wrote %d bytes for %s\n", n, fptr->f_name); //打印消息,发送到服务器的字节数,和文件的名字

	fptr->f_flags = F_READING;			/* clears F_CONNECTING 把文件的状态标志成员变为正在读取 */

	FD_SET(fptr->f_fd, &rset);			/* will read server's reply 在读描述符集中添加此文件的描述符 */
	if (fptr->f_fd > maxfd) //更新select的最大文件描述符参数
		maxfd = fptr->f_fd;
}
