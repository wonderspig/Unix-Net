#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;
	struct in_addr		**pptr;
	struct in_addr		*inetaddrp[2];
	struct in_addr		inetaddr;
	struct hostent		*hp;
	struct servent		*sp;

	if (argc != 3) //如果命令行参数不等于3,则报错退出
		err_quit("usage: daytimetcpcli1 <hostname> <service>");

	if ( (hp = gethostbyname(argv[1])) == NULL) { //第一个命令行参数应该为主机名,通过它获取hostent结构体
		//进入此分支代表gethostbyname出错,进入以下分支,手动获得地址
		if (inet_aton(argv[1], &inetaddr) == 0) { //把命令行第一个参数转换为二进制地址
			err_quit("hostname error for %s: %s", argv[1], hstrerror(h_errno));
		} else { //如果转换二进制地址正确,则进入以下分支
			inetaddrp[0] = &inetaddr; //把转换好的地址填入inetaddrp
			inetaddrp[1] = NULL; //数组其他成员置空
			//数组的每一个元素都是in_addr结构体的指针,所以指向数组的指针pptr是一个二级指针
			pptr = inetaddrp; //把填写好的数组inetaddrp附值给pptr
		}
	} else { //如果gethostbyname转换正确,则进入以下分支
		pptr = (struct in_addr **) hp->h_addr_list; //把成功获得的hp结构体中的地址列表对pptr附值
	}

	if ( (sp = getservbyname(argv[2], "tcp")) == NULL) //第二个命令行参数为服务名,通过它获得sp结构体
		err_quit("getservbyname error for %s", argv[2]);

	//循环遍历获得的地址列表,依次往servaddr.sin_addr填入每一个地址,这个地址用来connect连接服务器
	for ( ; *pptr != NULL; pptr++) { 
		sockfd = Socket(AF_INET, SOCK_STREAM, 0); //创建套接字,IP4,tcp协议

		bzero(&servaddr, sizeof(servaddr)); //清空地址结构体
		servaddr.sin_family = AF_INET; //IP4地址族
		servaddr.sin_port = sp->s_port; //填入端口号
		memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr)); //填入地址
		//打印正在尝试连接的地址信息
		printf("trying %s\n",
			   Sock_ntop((SA *) &servaddr, sizeof(servaddr))); //协议无关的转换二进制地址到字符串格式

		if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) == 0) //连接遍历到的这个地址
			break;		/* success 有一个地址成功,就跳出循环*/
		err_ret("connect error");
		close(sockfd);
	}
	if (*pptr == NULL) //没有取得有效地址
		err_quit("unable to connect");

	while ( (n = Read(sockfd, recvline, MAXLINE)) > 0) { //从服务器sockfd中读,存放入recvline中
		recvline[n] = 0;	/* null terminate 末尾加上NULL*/
		Fputs(recvline, stdout); //把从服务器读到的数据打印到标准输出
	}
	exit(0);
}
