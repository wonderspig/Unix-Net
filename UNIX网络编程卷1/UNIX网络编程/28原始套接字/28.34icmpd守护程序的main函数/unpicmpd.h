#ifndef	__unpicmp_h
#define	__unpicmp_h

#include	"unp.h"

#define	ICMPD_PATH		"/tmp/icmpd"	/* server's well-known pathname Unix域套接字的路径*/

//如果icmpd程序接收到ICMP错误,就把这个错误组成一个icmpd_err,发送给客户程序
struct icmpd_err {
  int				icmpd_errno;/* EHOSTUNREACH, EMSGSIZE, ECONNREFUSED 自定义的把ICMP错误映射成一个icmpd_errno值*/
  char				icmpd_type;	/* actual ICMPv[46] type ICMP类型*/
  char				icmpd_code;	/* actual ICMPv[46] code ICMP错误代码*/
  socklen_t			icmpd_len;	/* length of sockaddr{} that follows 套接字通用地址的长度*/
  struct sockaddr_storage	icmpd_dest;	/* sockaddr_storage handles any size 既能够存放IP4协议地址,又能够存放IP6协议地址的结构体*/
};

#endif	/* __unpicmp_h */
