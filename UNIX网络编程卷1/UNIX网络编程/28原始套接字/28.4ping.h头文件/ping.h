#include	"unp.h"
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include	<netinet/ip_icmp.h>

#define	BUFSIZE		1500

			/* globals */
char	 sendbuf[BUFSIZE];  //发送缓冲区

int		 datalen;			/* # bytes of data following ICMP header ICMP协议首部之后的数据区的长度*/
char	*host; //主机名
int		 nsent;				/* add 1 for each sendto() 第几次发送,每sendto发送一条ICMP消息,此序号+1*/
pid_t	 pid;				/* our PID 本进程pid*/
int		 sockfd;           //连接到对端的socket
int		 verbose;          //命令行参数-v的标志量,-v参数表示是否更详细的显示信息

			/* function prototypes */
void	 init_v6(void); //初始化ip6地址(包括IP6的套接字选项,过滤器等等设置)
void	 proc_v4(char *, ssize_t, struct msghdr *, struct timeval *); //处理IP4的函数
void	 proc_v6(char *, ssize_t, struct msghdr *, struct timeval *); //处理IP6的函数
void	 send_v4(void); //向IP4发送消息
void	 send_v6(void); //向IP6发送消息
void	 readloop(void); //循环读取对端的ICMP协议回应
void	 sig_alrm(int); //alarm信号处理函数(每一秒alarm函数都会发送一条ping的icmp消息)
void	 tv_sub(struct timeval *, struct timeval *); //计算传入的两个时间参数的时间差
//IP协议结构体
struct proto {
  void	 (*fproc)(char *, ssize_t, struct msghdr *, struct timeval *); //指向proc_v4或者proc_v6(处理所接收的ICMP消息)
  void	 (*fsend)(void); //指向send_v4或send_v6(发送ICMP消息)
  void	 (*finit)(void); //只有IP6时需要初始化,使用init_v6
  struct sockaddr  *sasend;	/* sockaddr{} for send, from getaddrinfo 用addrinfo确定的对端地址*/
  struct sockaddr  *sarecv;	/* sockaddr{} for receiving 用和发送地址的大小一样的地址用于接受*/
  socklen_t	    salen;		/* length of sockaddr{}s 套接字地址大小*/
  int	   	    icmpproto;	/* IPPROTO_xxx value for ICMP 指定协议为ICMP协议*/
} *pr;

#ifdef	IPV6

#include	<netinet/ip6.h>
#include	<netinet/icmp6.h>

#endif
