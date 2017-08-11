#include	"mesg.h"

void
server(int readfd, int writefd) //读写使用同一个消息队列标识符
{
	FILE	*fp;
	char	*ptr;
	pid_t	pid;
	ssize_t	n;
	struct mymesg	mesg; //消息队列结构体

	for ( ; ; ) {
			/* 4read pathname from IPC channel */
		mesg.mesg_type = 1; //把消息类型置为1,取消息为1的类型

		//从readfd中取出消息为1的类型，放置在mesg中(mesg中包含了待取出的消息类型的数据)
		//mesg中包含了待取出的消息类型的数据在消息取出之后,会替换成取出消息的消息类型
		if ( (n = Mesg_recv(readfd, &mesg)) == 0) {
			err_msg("pathname missing"); //数据长度等于0则报错
			continue;
		}
		//经过函数Mesg_recv从消息队列中取出消息之后,mesg的mesg_data成员中已经含有了消息的正文数据
		mesg.mesg_data[n] = '\0';	/* null terminate pathname 路径名要以空字符串结尾*/

		//mesg_data的格式是PID+空格+路径名

		if ( (ptr = strchr(mesg.mesg_data, ' ')) == NULL) { //查找字符串mesg.mesg_data中首次出现' '字符的位置
			err_msg("bogus request: %s", mesg.mesg_data); //没有' '字符,则报错
			continue;
		}
		//此时ptr指向PID和路径名之间的空格,ptr++之后,ptr就指向了路径名开始的第一个字符
		*ptr++ = 0;			/* null terminate PID, ptr = pathname */
		pid = atol(mesg.mesg_data); //atol函数会转换数值,直到空格为止,所以可以取出mesg.mesg_data中的pid
		/* for messages back to client 把pid放入消息队列的类型字段中，用来当返回客户端时的消息类型*/
		mesg.mesg_type = pid;

		//经过ptr++,现在的ptr指向路径名的开始处
		if ( (fp = fopen(ptr, "r")) == NULL) { //以读的方式打开ptr指向的路径名
			//进入分支,说明打开文件失败
				/* 4error: must tell client */

			//打印错误消息,保留客户端传过来的数据正文,服务器的错误提示附加在原数据之后
			snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n,
					 ": can't open, %s\n", strerror(errno));
			mesg.mesg_len = strlen(ptr);
			memmove(mesg.mesg_data, ptr, mesg.mesg_len);
			Mesg_send(writefd, &mesg); //告诉客户端，该路径无法打开

		} else {
			//进入此分支代表打开路径成功
				/* 4fopen succeeded: copy file to IPC channel */
			while (Fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) { //每次一行的读出fp文件中的数据
				mesg.mesg_len = strlen(mesg.mesg_data); //计算一下读出的这一行的长度,填入mesg.mesg_len中
				//把读出的数据写入消息队列(消息队列函数需要的类型字段,数据长度字段等等,都在mesg中)
				Mesg_send(writefd, &mesg);
			}
			Fclose(fp); //关闭文件
		}

			/* 4send a 0-length message to signify the end */
		mesg.mesg_len = 0;//写入一个长度为0的消息,代表发送结束
		Mesg_send(writefd, &mesg); //发送消息长度为0的消息
	}
}
