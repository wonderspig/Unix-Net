#include	"mesg.h"

void
client(int readfd, int writefd)
{
	size_t	len;
	ssize_t	n;
	char	*ptr;
	struct mymesg	mesg;

		/* 4start buffer with pid and a blank */
	//mesg.mesg_data的格式为"PID+空格+路径名"
	snprintf(mesg.mesg_data, MAXMESGDATA, "%ld ", (long) getpid()); //在mesg_data的开头处放入本进程的pid
	len = strlen(mesg.mesg_data);//计算一下pid的长度(包括末尾的空白字符)
	ptr = mesg.mesg_data + len;//把指针ptr指向pid+"\0"之后的空白处,等于已经包含了空格

		/* 4read pathname */
	//从标准输入读入一行,最大读取的数据长度不能超过消息队列允许的最大数据长度减去pid已经是用掉的数据长度
	Fgets(ptr, MAXMESGDATA - len, stdin);
	len = strlen(mesg.mesg_data); //计算一下此时mesg.mesg_data的长度
	if (mesg.mesg_data[len-1] == '\n') //判断是否是以换行符结尾的
		len--;				/* delete newline from fgets() 是换行符结尾就删除换行符 */
	mesg.mesg_len = len; //最终确认数据的长度
	mesg.mesg_type = 1; //消息类型置为1

		/* 4write PID and pathname to IPC channel */
	Mesg_send(writefd, &mesg); //放入消息队列

		/* 4read from IPC, write to standard output */
	mesg.mesg_type = getpid(); //把本进程的pid填入mesg.mesg_type,用来从消息队列中读取服务器返回的应答
	while ( (n = Mesg_recv(readfd, &mesg)) > 0) //从消息队列中读取以本进程pid为消息类型字段的消息
		Write(STDOUT_FILENO, mesg.mesg_data, n); //把消息队列中读取出来的消息写到标准输出
}
