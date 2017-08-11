#include	"mesg.h"

void
client(int readid, int writeid)
{
	size_t	len;
	ssize_t	n;
	char	*ptr;
	struct mymesg	mesg;

		/* 4start buffer with msqid and a blank */
	snprintf(mesg.mesg_data, MAXMESGDATA, "%d ", readid); //把自己创建的私有消息队列的id传送给服务器
	len = strlen(mesg.mesg_data); //计算一下当前已经保存readid的mesg_data的长度(包含'\0')
	ptr = mesg.mesg_data + len; //ptr指向readid+空格之后的位置

		/* 4read pathname */
	Fgets(ptr, MAXMESGDATA - len, stdin); //从标准输入读入一行,放入prt指向readid+空格之后的位置,从这个位置开始存放
	len = strlen(mesg.mesg_data); //重新计算一下当前mesg_data的长度(里面已经包括readid+空格+标准输入读入的路径)
	if (mesg.mesg_data[len-1] == '\n') //删除掉标准输入里读入的'\n'
		len--;				/* delete newline from fgets() */
	mesg.mesg_len = len; //删除掉\n的len的长度(正文数据的确切长度)
	mesg.mesg_type = 1; //消息类型置为1

		/* 4write msqid and pathname to server's well-known queue */
	Mesg_send(writeid, &mesg); //把消息放入消息队列,mesg中有消息正文的长度,消息类型,正文数据

		/* 4read from our queue, write to standard output */
	//从服务器读取应答
	//(因为每个客户程序都有自己的消息队列,所以不再需要指定特别的消息类型，这个消息队列中的所有消息,都是本客户进程的)
	while ( (n = Mesg_recv(readid, &mesg)) > 0)
		Write(STDOUT_FILENO, mesg.mesg_data, n); //把消息队列中取出的数据,写入标准输出
}
