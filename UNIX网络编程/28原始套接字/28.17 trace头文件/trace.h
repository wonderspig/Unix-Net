#include	"unp.h"
#include	<netinet/in_systm.h>
#include	<netinet/ip.h>
#include	<netinet/ip_icmp.h>
#include	<netinet/udp.h>

#define	BUFSIZE		1500 //缓冲区大小
//发送的UDP包的数据部分
struct rec {					/* format of outgoing UDP data */
  u_short	rec_seq;			/* sequence number UDP包的序列号*/
  u_short	rec_ttl;			/* TTL packet left with 跳限*/
  struct timeval	rec_tv;		/* time packet left 发送的时间点*/
};

			/* globals */
char	 recvbuf[BUFSIZE]; //接收缓冲区
char	 sendbuf[BUFSIZE]; //发送缓冲区

int		 datalen;			/* # bytes of data following ICMP header ICMP首部后跟的数据长度*/
char	*host; //主机名
u_short	 sport, dport; //源端口 目的端口
int		 nsent;				/* add 1 for each sendto() 计数: 发送了多少个UDP数据包 */
pid_t	 pid;				/* our PID 本进程的PID*/
int		 probe, nprobes; //nprobes:每个跳限探测多少次  probe:本次发送是多少次探测
int		 sendfd, recvfd;	/* send on UDP sock, read on raw ICMP sock 发送描述符是UDP套接字,接收描述符是ICMP原始套接字*/
int		 ttl, max_ttl; //跳限, 最大跳限
int		 verbose; //判断命令行参数有没有-v参数的标志量

			/* function prototypes */
const char	*icmpcode_v4(int); //处理ICMP4的错误消息代码
const char	*icmpcode_v6(int); //处理ICMP6的错误消息代码
int		 recv_v4(int, struct timeval *); //读入并处理ICMP4消息
int		 recv_v6(int, struct timeval *); //读入并处理ICMP6消息
void	 sig_alrm(int); //信号处理函数
void	 traceloop(void); //程序主循环
void	 tv_sub(struct timeval *, struct timeval *); //两个时间点之差存放在前一个变量中

//proto结构体将会被初始化为ICMP4和ICMP6各一份
struct proto {
  const char	*(*icmpcode)(int); //处理ICMP的错误消息代码
  int	 (*recv)(int, struct timeval *); //读入并处理ICMP消息
  struct sockaddr  *sasend;	/* sockaddr{} for send, from getaddrinfo 从addrinfo中查找出来的对端的地址*/
  struct sockaddr  *sarecv;	/* sockaddr{} for receiving 将会用对端地址相同的长度,来初始化这个地址*/
  struct sockaddr  *salast;	/* last sockaddr{} for receiving 最后一个收到的数据包的对端地址*/
  struct sockaddr  *sabind;	/* sockaddr{} for binding source port 这个地址是用来让UDP协议绑定源端口的*/
  socklen_t   		salen;	/* length of sockaddr{}s 用来存放地址的长度*/
  int			icmpproto;	/* IPPROTO_xxx value for ICMP ICMP的协议常量*/
  int	   ttllevel;		/* setsockopt() level to set TTL 设置ttl是,setsockopt函数需要的level参数*/
  int	   ttloptname;		/* setsockopt() name to set TTL 设置ttl是,setsockopt函数需要的name参数*/
} *pr;

#ifdef	IPV6 //根据是否支持IPv6协议来决定,是否包含IPv6的头文件

#include	<netinet/ip6.h>
#include	<netinet/icmp6.h>

#endif
