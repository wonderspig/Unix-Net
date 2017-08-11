#include	"mesg.h"

void
server(int readfd, int writefd)
{
	FILE	*fp;
	ssize_t	n;
	struct mymesg	mesg;

		/* 4read pathname from IPC channel */
	mesg.mesg_type = 1; //结构体的类型成员填写入1
	if ( (n = Mesg_recv(readfd, &mesg)) == 0) //从客户端读入请求
		err_quit("pathname missing");
	mesg.mesg_data[n] = '\0';	/* null terminate pathname 把从客户进程接受的请求调整为以'\0'结尾 */

	if ( (fp = fopen(mesg.mesg_data, "r")) == NULL) { //打开客户进程所要求的文件
			/* 4error: must tell client */
		snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n,
				 ": can't open, %s\n", strerror(errno)); //打开失败,则发送这条失败消息给客户进程
		mesg.mesg_len = strlen(mesg.mesg_data);
		Mesg_send(writefd, &mesg);

	} else { //打开文件成功进入此分支
			/* 4fopen succeeded: copy file to IPC channel */
		while (Fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) { //从已经打开的文件中读取一行
			mesg.mesg_len = strlen(mesg.mesg_data); //计算读读取到的这一行的长度
			Mesg_send(writefd, &mesg); //把这一行发送给客户程序
		}
		Fclose(fp);
	}

		/* 4send a 0-length message to signify the end */
	mesg.mesg_len = 0;
	Mesg_send(writefd, &mesg); //发送长度为0的消息,代表服务器已经提供完了所要求的服务
}
