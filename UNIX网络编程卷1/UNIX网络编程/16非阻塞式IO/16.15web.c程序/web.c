/* include web1 */
#include	"web.h"

int
main(int argc, char **argv)
{
	socklen_t n;
	int		i, fd, maxnconn, flags, error;
	char	buf[MAXLINE];
	fd_set	rs, ws;
	//参数1:最大连接数 参数2:主机名 参数3:主机目录 参数4:下载的第一个文件 参数5:下载的第二个文件
	if (argc < 5) //如果命令行参数小于四个,则命令格式不对，出错退出
		err_quit("usage: web <#conns> <hostname> <homepage> <file1> ...");
	maxnconn = atoi(argv[1]); //第一参数为最大并行连接数,转换成int格式后附值给maxnconn

	//待打开的文件数是从命令行的第五个参数开始,所以待打开的文件数等于命令行的总的参数个数-4
	nfiles = min(argc - 4, MAXFILES); 
	for (i = 0; i < nfiles; i++) { //用循环遍历每一个命行参数传递进来文件名
		file[i].f_name = argv[i + 4]; //第一个文件名在命令行参数中的位置是1+4,第二个2+4……
		file[i].f_host = argv[2]; //文件所在的主机名或IP地址为,命令行第二个参数
		file[i].f_flags = 0; //文件标志初始化为0
	}
	printf("nfiles = %d\n", nfiles); //打印一下有多少个文件需要被读取

	home_page(argv[2], argv[3]); //命令行第二个参数为服务器的主机名,第三个参数为服务器的路径名,已阻塞的方式连接服务器

	//清空读写两个描述符集
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	maxfd = -1; //调用select的最大文件描述符数,初始化为-1
	//nlefttoread仍待读取的文件数,nlefttoconn尚无TCP连接的文件数
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0; //nconn当前打开着的连接数
/* end web1 */
/* include web2 */
	while (nlefttoread > 0) { //只要还有待处理的文件，就进行循环
		while (nconn < maxnconn && nlefttoconn > 0) { //如果当前打开的连接数,小于最大能连接的连接数,并且待连接数大于0
				/* 4find a file to read */
			for (i = 0 ; i < nfiles; i++) //在所有待打开的文件里循环(查找连连接都没有建立的文件)
				if (file[i].f_flags == 0) //文件的状态标志成员为0(说明连连接都没开始,则跳出循环,将会对此文件进行connect)
					break;
			if (i == nfiles) /* 循环中的i和总共需要打开的文件相同,说明循环没有找到任何一个flag成员不等于0的文件
							    所有文件至少都已经开始建立连接 */
				err_quit("nlefttoconn = %d but nothing found", nlefttoconn);
			start_connect(&file[i]); //对找到的这个连连接都没有建立的文件进行非阻塞的connect,如果connect成功,则发送服务器请求命令	
									 //此函数产生两种描述符,一种为connect的三路握手中,此种描述符分别加入读描述符集,和写描述符集
									 //第二种描述符为connect成功,write_get_cmd这个函数会向服务器写入命令,等待服务器响应,此种描述符
									 //会加入读描述符集
			nconn++; //当前打开的连接数+1
			nlefttoconn--; //尚无TCP连接的的文件数-1
		}

		//把读写描述符集各拷贝一份,select会修改描述符集,这样可以防止被破坏
		rs = rset; 
		ws = wset;
		//最大描述符集会在start_connect这个函数中更新,
		n = Select(maxfd+1, &rs, &ws, NULL, NULL); //调用select,等待两种描述符
												   //1、connect的连接完成,2、或者有描述符可读(已发命令给服务器,等待服务器的响应)

		for (i = 0; i < nfiles; i++) { //遍历所有文件
			flags = file[i].f_flags;
			if (flags == 0 || flags & F_DONE) //判断文件结构体的标志成员为0(还没开始链接,待处理),或该文件已经下载完毕
				continue; //继续循环
			fd = file[i].f_fd; //取出该文件的fd(是在start_connect函数中放入文件结构体的)
			//第一种情况的描述符
			if (flags & F_CONNECTING &&
				(FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws))) { //文件结构体标志成员为正在连接,并且可读或者可写(此文件可能刚刚connect成功)
				n = sizeof(error);
				if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &n) < 0 ||
					error != 0) { //取一下该套接字中的error值
					err_ret("nonblocking connect failed for %s",
							file[i].f_name);
				}
				//代表error的值为0,证明此套接字确实刚刚connetc成功
					/* 4connection established */
				printf("connection established for %s\n", file[i].f_name); //打印信息
				FD_CLR(fd, &wset);		/* no more writeability test 把改描述符从可写描述符集中删除 */
				write_get_cmd(&file[i]);/* write() the GET command 发送命令给服务器,并且向服务器发送请求后,把描述符加入可读描述符集*/

			//第二种情况的描述符
			} else if (flags & F_READING && FD_ISSET(fd, &rs)) { //文件标志成员为正在读取,且描述符可读(说明服务器的响应到达了)
				if ( (n = Read(fd, buf, sizeof(buf))) == 0) { //从服务器中读取
					printf("end-of-file on %s\n", file[i].f_name); //读到eof(读完了)
					Close(fd); //关闭这个连接到服务器的套接字
					file[i].f_flags = F_DONE;	/* clears F_READING 文件结构体的标志成员设置为已完成*/
					FD_CLR(fd, &rset); //清除可读描述符集中此描述符的对应位
					nconn--; //已连接的描述符数-1
					nlefttoread--; //待处理的文件数-1
				} else {
					printf("read %d bytes from %s\n", n, file[i].f_name); //正确读取，打印消息
				}
			}
		}
	}
	exit(0);
}
/* end web2 */
