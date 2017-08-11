#include	"mesg.h"

void
server(int readid, int writeid)
{
	FILE	*fp;
	char	*ptr;
	ssize_t	n;
	struct mymesg	mesg;
	void	sig_chld(int);

	Signal(SIGCHLD, sig_chld); //注册信号处理函数

	for ( ; ; ) {
			/* 4read pathname from our well-known queue */
		mesg.mesg_type = 1; //消息类型置为1,读取消息队列中消息类型为1的消息
		//从readfd中取出消息为1的类型，放置在mesg中(mesg中包含了待取出的消息类型的数据)
		//mesg中包含了待取出的消息类型的数据在消息取出之后,会替换成取出消息的消息类型
		if ( (n = Mesg_recv(readid, &mesg)) == 0) { //从消息队列中读取消息,放入mesg中
			err_msg("pathname missing"); //消息长度为0,则报错
			continue;
		}
		//经过函数Mesg_recv从消息队列中取出消息之后,mesg的mesg_data成员中已经含有了消息的正文数据
		mesg.mesg_data[n] = '\0';	/* null terminate pathname */

		//mesg_data的格式是readid+空格+路径名

		if ( (ptr = strchr(mesg.mesg_data, ' ')) == NULL) { //查找字符串mesg.mesg_data中首次出现' '字符的位置
			err_msg("bogus request: %s", mesg.mesg_data); //没有' '字符,则报错
			continue;
		}
		//此时ptr指向readid和路径名之间的空格,ptr++之后,ptr就指向了路径名开始的第一个字符
		*ptr++ = 0;			/* null terminate msgid, ptr = pathname */
		writeid = atoi(mesg.mesg_data); //atol函数会转换数值,直到空格为止,所以可以取出mesg.mesg_data中的readid

		if (Fork() == 0) {		/* child 创建一个子进程,用来回应客户的请求*/
			if ( (fp = fopen(ptr, "r")) == NULL) { //以读的方式打开ptr指向的路径名
				//进入分支,说明打开文件失败
					/* 4error: must tell client */
				//打印错误消息,保留客户端传过来的数据正文,服务器的错误提示附加在原数据之后
				snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n,
						 ": can't open, %s\n", strerror(errno));
				mesg.mesg_len = strlen(ptr);
				memmove(mesg.mesg_data, ptr, mesg.mesg_len);
				//告诉客户端，该路径无法打开
				Mesg_send(writeid, &mesg);

			} else {
				//进入此分支代表打开路径成功
					/* 4fopen succeeded: copy file to client's queue */
				while (Fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) { //每次一行的读出fp文件中的数据
					mesg.mesg_len = strlen(mesg.mesg_data); //计算一下读出的这一行的长度,填入mesg.mesg_len中
					//把读出的数据写入消息队列(消息队列函数需要的类型字段,数据长度字段等等,都在mesg中)
					Mesg_send(writeid, &mesg);
				}
				Fclose(fp); //关闭已经打开的文件
			}

				/* 4send a 0-length message to signify the end */
			mesg.mesg_len = 0; //写入一个长度为0的消息,代表发送结束
			Mesg_send(writeid, &mesg); //发送消息长度为0的消息
			exit(0);		/* child terminates */
		}
		/* parent just loops around */
	}
}
