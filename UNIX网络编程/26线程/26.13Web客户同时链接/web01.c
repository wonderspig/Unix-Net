/* include web1 */
#include	"unpthread.h"
#include	<thread.h>		/* Solaris threads */

#define	MAXFILES	20 //最大文件数
#define	SERV		"80"	/* port number or service name 端口号 */
//文件结构体
struct file {
  char	*f_name;			/* filename 文件名 */
  char	*f_host;			/* hostname or IP address 主机名或IP地址 */
  int    f_fd;				/* descriptor 文件描述符 */
  int	 f_flags;			/* F_xxx below 标志,当前的文件状态 */
  pthread_t	 f_tid;			/* thread ID 线程ID */
} file[MAXFILES]; 
#define	F_CONNECTING	1	/* connect() in progress 文件正在连接状态 */
#define	F_READING		2	/* connect() complete; now reading 文件正在读取状态 */
#define	F_DONE			4	/* all done 文件处理完成 */

#define	GET_CMD		"GET %s HTTP/1.0\r\n\r\n" //发送服务器的格式字符串

int		nconn, nfiles, nlefttoconn, nlefttoread;

void	*do_get_read(void *);
void	home_page(const char *, const char *);
void	write_get_cmd(struct file *);

int
main(int argc, char **argv)
{
	int			i, n, maxnconn;
	pthread_t	tid;
	struct file	*fptr;
	//参数1:最大连接数 参数2:主机名 参数3:主机目录 参数4:下载的第一个文件 参数5:下载的第二个文件
	if (argc < 5) //命令行参数小于4个,则格式不对，出错退出
		err_quit("usage: web <#conns> <IPaddr> <homepage> file1 ...");
	maxnconn = atoi(argv[1]); //把第一个参数,最大连接数从字符串格式转换成int格式

	nfiles = min(argc - 4, MAXFILES); //文件名的数量:取参数给出的文件名数量,如果超过了支持的最大文件数量,则取最大文件数量
	for (i = 0; i < nfiles; i++) { //对已经确定的文件名数量进行循环
		file[i].f_name = argv[i + 4]; //对文件名赋值
		file[i].f_host = argv[2]; //对主机名或IP地址赋值
		file[i].f_flags = 0; //标志(状态)清空为0
	}
	printf("nfiles = %d\n", nfiles); //打印一下文件数量
	//测试服务器是否存在
	home_page(argv[2], argv[3]); //根据主机名和文件路径,查找服务器地址,并连接到服务器,发送一格请求,并读取请求,再关闭连接到服务器的套接字
	//初始化"仍待读取的文件数"和"尚无TCP连接的文件数"等于"总共的文件数"
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0; //当前打开着的连接数
/* end web1 */
/* include web2 */
	while (nlefttoread > 0) { //仍待读取的文件数大于0
		while (nconn < maxnconn && nlefttoconn > 0) { //当前打开着的连接数小于最大能打开的连接数,且尚无TCP连接的文件数大于0
				/* 4find a file to read */
			for (i = 0 ; i < nfiles; i++) //便利所有文件
				if (file[i].f_flags == 0) //查找标志为0(当前没有处理过的文件)
					break; //找不到则跳出循环
			if (i == nfiles) //如果i等于最大文件数,说明便利了循环没有找到
				err_quit("nlefttoconn = %d but nothing found", nlefttoconn);

			file[i].f_flags = F_CONNECTING; //标志改为正在连接
			Pthread_create(&tid, NULL, &do_get_read, &file[i]); //创建线程,线程属性默认,把文件信息结构体当参数传入
			file[i].f_tid = tid; //把线程id存入文件信息结构体
			nconn++; //当前连接数+1
			nlefttoconn--; //尚无TCP连接的文件数-1
		}

		if ( (n = thr_join(0, &tid, (void **) &fptr)) != 0)
			errno = n, err_sys("thr_join error");

		nconn--; //当前连接数-1
		nlefttoread--; //仍待读取的文件数-1
		printf("thread id %d for %s done\n", tid, fptr->f_name);
	}

	exit(0);
}
/* end web2 */

/* include do_get_read */
void *
do_get_read(void *vptr)
{
	int					fd, n;
	char				line[MAXLINE];
	struct file			*fptr;

	fptr = (struct file *) vptr; //把传递给线程的参数强转成file*

	fd = Tcp_connect(fptr->f_host, SERV); //根据主机名和端口号连接到服务器
	fptr->f_fd = fd; //把连接到服务器的套接字放入文件信息结构体
	printf("do_get_read for %s, fd %d, thread %d\n",
			fptr->f_name, fd, fptr->f_tid);

	write_get_cmd(fptr);	/* write() the GET command 这个函数用来向服务器发送格式化好的数据,并把文件标志改为待读取的文件*/

		/* 4Read server's reply */
	for ( ; ; ) {
		if ( (n = Read(fd, line, MAXLINE)) == 0) //从服务器处读取应答
			break;		/* server closed connection 读到EOF,已经读完了应答*/

		printf("read %d bytes from %s\n", n, fptr->f_name);
	}
	printf("end-of-file on %s\n", fptr->f_name);
	Close(fd); //读完了文件关闭连接到服务器的描述符
	fptr->f_flags = F_DONE;		/* clears F_READING 把文件标志修改为已完成 */

	return(fptr);		/* terminate thread 返回文件信息结构体的指针 */
}
/* end do_get_read */

/* include write_get_cmd */
void
write_get_cmd(struct file *fptr)
{
	int		n;
	char	line[MAXLINE];

	n = snprintf(line, sizeof(line), GET_CMD, fptr->f_name); //组合发送给服务器的表达式,存入缓冲line中
	Writen(fptr->f_fd, line, n); //把line缓冲区发送到服务器
	printf("wrote %d bytes for %s\n", n, fptr->f_name);

	fptr->f_flags = F_READING;			/* clears F_CONNECTING 把正在连接标志改为待读取标志*/
}
/* end write_get_cmd */

/* include home_page */
void
home_page(const char *host, const char *fname)
{
	int		fd, n;
	char	line[MAXLINE];

	fd = Tcp_connect(host, SERV);	/* blocking connect() 根据命令行的主机名和中所周知的端口号连接服务器 */

	n = snprintf(line, sizeof(line), GET_CMD, fname); //生成服务起的命令格式,存放在line中
	Writen(fd, line, n); //把命令发送给服务器

	for ( ; ; ) {
		if ( (n = Read(fd, line, MAXLINE)) == 0) //读取服务器应答
			break;		/* server closed connection 读到EO则跳出循环*/

		printf("read %d bytes of home page\n", n);
		/* do whatever with data */
	}
	printf("end-of-file on home page\n");
	Close(fd); //关闭连接到服务器的套接字
}
/* end home_page */
