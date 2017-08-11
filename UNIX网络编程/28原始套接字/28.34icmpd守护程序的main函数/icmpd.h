#include	"unpicmpd.h"

//保存客户端信息的结构体
struct client {
  int	connfd;			/* Unix domain stream socket to client 连接到客户端的Unix域套接字*/
  int	family;			/* AF_INET or AF_INET6 客户UDP套接字的地址族 */
  int	lport;			/* local port bound to client's UDP socket 绑定在UDP套接字上的端口号*/
						/* network byte ordered */
} client[FD_SETSIZE]; //client结构体数组

					/* 4globals */
				//fd4:IP4协议的ICMP描述符  
				//fd6:IP6协议的ICMP描述符
				//listenfd:监听描述符
				//nready:Select的返回值,代表有多少描述符准备好了,每处理一个描述符,就把这个全局变量递减1,处理完了,再继续Select
				//maxi:client结构体数组中的已经被使用的元素个数
				//maxfd:Select的最大描述符个数
int				fd4, fd6, listenfd, maxi, maxfd, nready;
fd_set			rset, allset; //描述符集allset:用来记录需要关系的描述符
							  //把描述符集allset中的描述符复制到rset中之后,把rset描述符集放入Select中
							  //(Select可能会改变描述符集)
struct sockaddr_un	cliaddr; //Unix域地址结构体

			/* 4function prototypes */
		//处理各种类型的描述符的函数
int		 readable_conn(int); 
int		 readable_listen(void);
int		 readable_v4(void);
int		 readable_v6(void);
