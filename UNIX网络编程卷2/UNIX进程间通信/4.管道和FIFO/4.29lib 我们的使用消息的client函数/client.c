#include	"mesg.h"

void
client(int readfd, int writefd)
{
	size_t	len;
	ssize_t	n;
	struct mymesg	mesg;

		/* 4read pathname */
	Fgets(mesg.mesg_data, MAXMESGDATA, stdin); //从标准输入读取数据,数据的最大长度不能超过定义的最大正文数据长度
	len = strlen(mesg.mesg_data); //计算一下当前的数据长度
	if (mesg.mesg_data[len-1] == '\n') //如果缓冲区中的数据是以'\n'结尾的
		len--;				/* delete newline from fgets() 删除这个'\n' */
	mesg.mesg_len = len; //把除去'\n'的长度填写入结构体的正文数据长度成员
	mesg.mesg_type = 1; //填写结构体中数据的类型成员

		/* 4write pathname to IPC channel */
	Mesg_send(writefd, &mesg); //把填写完成的结构体发送到用于写的FIFO中(传送给服务器)

		/* 4read from IPC, write to standard output */
	while ( (n = Mesg_recv(readfd, &mesg)) > 0) //循环读取服务器的应答,并复制到标准输出
		Write(STDOUT_FILENO, mesg.mesg_data, n);
}
