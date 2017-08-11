#include "unp.h"
#include <net/pfkeyv2.h>

void
print_sadb_msg(struct sadb_msg *msg, int msglen);
int
getsatypebyname(char *name);
int
getsaalgbyname(int type, char *name);

int
salen(struct sockaddr *sa)  //根据通用地址结构体,计算出这个地址的长度
{
#ifdef HAVE_SOCKADDR_SA_LEN
	return sa->sa_len;
#else
	switch (sa->sa_family) {
	case AF_INET:
		return sizeof(struct sockaddr_in);
#ifdef IPV6
	case AF_INET6:
		return sizeof(struct sockaddr_in6);
#endif
	default:
		return 0;	/* XXX */
	}
#endif
}

int
prefix_all(struct sockaddr *sa) //根据通用地址结构体,计算出地址的位数(多少位地址)
{
	switch (sa->sa_family) {
	case AF_INET:
		return 32;
#ifdef IPV6
	case AF_INET6:
		return 128;
#endif
	default:
		return 0;	/* XXX */
	}
}

/* include sadb_add */
void
sadb_add(struct sockaddr *src, struct sockaddr *dst, int type, int alg,
		int spi, int keybits, unsigned char *keydata)
{
	int s;
	char buf[4096], *p;	/* XXX 定义一个缓冲区,这个缓冲区中会依次存放msg首部,sa扩展首部,地址扩展首部,秘钥扩展首部 */
	struct sadb_msg *msg; //秘钥管理套接字首部
	struct sadb_sa *saext; //SA扩展首部
	struct sadb_address *addrext; //地址扩展首部
	struct sadb_key *keyext; //秘钥扩展首部
	int len; //整个秘钥消息的长度(包括msg首部,SA扩展首部,地址扩展首部,秘钥扩展首部的总和长度)
	int mypid; //本进程的pid

	s = Socket(PF_KEY, SOCK_RAW, PF_KEY_V2); //创建秘钥管理套接字,版本为PF_KEY_V2

	mypid = getpid(); //保存一下本进程的pid

	//首部的长度计算按照8个字节对齐,也就是存储在结构体中的数字n,代表有n个8个字节的长度

	/* Build and write SADB_ADD request */
	bzero(&buf, sizeof(buf)); //把缓冲区清零
	p = buf; //指针p指向缓冲区开始位置
	msg = (struct sadb_msg *)p; //缓冲区开始位置存放的是msg,秘钥消息首部
	msg->sadb_msg_version = PF_KEY_V2; //秘钥管理消息版本
	msg->sadb_msg_type = SADB_ADD; //秘钥管理消息类型
	msg->sadb_msg_satype = type; //SA类型: SADB_SATYPE_AH或者SADB_SATYPE_ESP
	msg->sadb_msg_pid = getpid(); //本进程的pid
	len = sizeof(*msg); //len的长度是msg首部的长度
	p += sizeof(*msg); //指针p指向msg首部后的一个位置

	saext = (struct sadb_sa *)p; //存放在msg首部之后的第一个扩展首部的SA扩展首部
	saext->sadb_sa_len = sizeof(*saext) / 8; //SA扩展首部的长度(代表: 有n个8个字节)
	saext->sadb_sa_exttype = SADB_EXT_SA; //指明是SA扩展首部
	saext->sadb_sa_spi = htonl(spi); //标识唯一的SA
	//重放窗口大笑,静态生成秘钥无法重放
	saext->sadb_sa_replay = 0;	/* no replay protection with static keys */
	saext->sadb_sa_state = SADB_SASTATE_MATURE; //动态秘钥生成期间的状态变化(静态秘钥始终是SADB_SASTATE_MATURE)
	saext->sadb_sa_auth = alg; //认证算法
	saext->sadb_sa_encrypt = SADB_EALG_NONE; //加密算法
	saext->sadb_sa_flags = 0; //静态秘钥直接置0
	len += saext->sadb_sa_len * 8; //维护len的长度为:msg首部+SA扩展首部
	p += saext->sadb_sa_len * 8; //指针p指向SA扩展首部后的一个位置

	addrext = (struct sadb_address *)p; //存放在SA扩展首部之后的是源地址扩展首部
	addrext->sadb_address_len = (sizeof(*addrext) + salen(src) + 7) / 8; //扩展首部的大小是:源地址扩展首部长度+地址长度(保证8对齐)
	addrext->sadb_address_exttype = SADB_EXT_ADDRESS_SRC; //扩展首部的类型为源地址扩展首部
	addrext->sadb_address_proto = 0;	/* any protocol 协议匹配为任何协议*/
	addrext->sadb_address_prefixlen = prefix_all(src); //地址的有效位数(IP4是32位,IP6是128位)
	addrext->sadb_address_reserved = 0; //存储延长期限
	memcpy(addrext + 1, src, salen(src)); //把通用地址格式中的地址,复制到源地址扩展首部之后的位置
	len += addrext->sadb_address_len * 8; //维护len的长度是:msg首部+SA扩展首部+源地址扩展首部+源地址
	p += addrext->sadb_address_len * 8; //p指向源地址扩展首部+源地址的后一个位置

	addrext = (struct sadb_address *)p; //存放在"源地址扩展首部+源地址"之后的是目的地址扩展首部
	addrext->sadb_address_len = (sizeof(*addrext) + salen(dst) + 7) / 8; //扩展首部的大小是:源地址扩展首部长度+地址长度(保证8对齐)
	addrext->sadb_address_exttype = SADB_EXT_ADDRESS_DST; //扩展首部的类型为目的地址扩展首部
	addrext->sadb_address_proto = 0;	/* any protocol 协议匹配为任何协议*/
	addrext->sadb_address_prefixlen = prefix_all(dst); //地址的有效位数(IP4是32位,IP6是128位)
	addrext->sadb_address_reserved = 0; //存储延长期限
	memcpy(addrext + 1, dst, salen(dst)); //把通用地址格式中的地址,复制到目的地址扩展首部之后的位置
	len += addrext->sadb_address_len * 8; //维护len的长度是:msg首部+SA扩展首部+源地址扩展首部+源地址+目的地址扩展首部+目的地址
	p += addrext->sadb_address_len * 8; //p指向目的地址扩展首部+目的地址的后一个位置

	keyext = (struct sadb_key *)p; //存放在"目的地址扩展首部+目的地址"之后的是秘钥扩展首部
	/* "+7" handles alignment requirements */
	//扩展首部的大小是:秘钥扩展首部的长度+秘钥的长度(保证8对齐)
	keyext->sadb_key_len = (sizeof(*keyext) + (keybits / 8) + 7) / 8; //keybits表示的是bits,除以8之后才是字节数
	keyext->sadb_key_exttype = SADB_EXT_KEY_AUTH; //秘钥扩展首部的类型是:身份认证秘钥
	keyext->sadb_key_bits = keybits; //秘钥的长度(bits)
	keyext->sadb_key_reserved = 0; //存储延长期限
	memcpy(keyext + 1, keydata, keybits / 8); //把秘钥复制到秘钥扩展首部之后的位置
	len += keyext->sadb_key_len * 8; //维护len的长度为:msg首部+SA扩展首部+源地址扩展首部+源地址+目的地址扩展首部+目的地址
														//＋秘钥扩展首部+秘钥
	p += keyext->sadb_key_len * 8; //p指向"秘钥扩展首部+秘钥"的后一个位置

	msg->sadb_msg_len = len / 8; //msg首部的长度成员为:len的长度
	printf("Sending add message:\n");
	print_sadb_msg((struct sadb_msg *)buf, len); //打印一下当前的buf缓冲区中的数据
	Write(s, buf, len); //把buf缓冲区写入秘钥套接字

	printf("\nReply returned:\n"); //开始用循环接受内核的返回消息
	/* Read and print SADB_ADD reply, discarding any others */
	for (;;) {
		int msglen; //内核返回消息的总长度(read函数的返回值)
		struct sadb_msg *msgp; //秘钥管理消息首部结构体

		msglen = Read(s, &buf, sizeof(buf)); //用read函数从内核读取应答
		msgp = (struct sadb_msg *)&buf; //把缓冲区转换成秘钥管理消息首部msg,从而开始读取内核返回的消息
		if (msgp->sadb_msg_pid == mypid && //如果内核返回消息的pid和本进程pid一致
			msgp->sadb_msg_type == SADB_ADD) { //钥管理消息首部msg的类型是SADB_ADD(也就是我们刚刚发送的类型)
			print_sadb_msg(msgp, msglen); //打印这个类型
			break; //跳出循环
		}
	}
	close(s); //关闭秘钥管理套接字
}
/* end sadb_add */

int
main(int argc, char **argv)
{
	struct addrinfo hints, *src, *dst;
	unsigned char *p, *keydata, *kp;
	char *ep;
	int ret, len, i;
	int satype, alg, keybits;

	bzero(&hints, sizeof(hints));
	if ((ret = getaddrinfo(argv[1], NULL, &hints, &src)) != 0) {
		err_quit("%s: %s\n", argv[1], gai_strerror(ret));
	}
	if ((ret = getaddrinfo(argv[2], NULL, &hints, &dst)) != 0) {
		err_quit("%s: %s\n", argv[2], gai_strerror(ret));
	}
	if (src->ai_family != dst->ai_family) {
		err_quit("%s and %s not same addr family\n", argv[1], argv[2]);
	}
	satype = SADB_SATYPE_AH;
	if ((alg = getsaalgbyname(satype, argv[3])) < 0) {
		err_quit("Unknown SA type / algorithm pair ah/%s\n", argv[3]);
	}
	keybits = strtoul(argv[4], &ep, 0);
	if (ep == argv[4] || *ep != '\0' || (keybits % 8) != 0) {
		err_quit("Invalid number of bits %s\n", argv[4]);
	}
	p = (unsigned char *)argv[5];
	if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
		p += 2;
	len = strlen((const char *)p);
	kp = keydata = malloc(keybits / 8);
	for (i = 0; i < keybits; i += 8) {
		int c;

		if (len < 2) {
			err_quit("%s: not enough bytes (expected %d)\n", argv[5], keybits / 8);
		}
		if (sscanf((const char *)p, "%2x", &c) != 1) {
			err_quit("%s contains invalid hex digit\n", argv[5]);
		}
		*kp++ = c;
		p += 2;
		len -= 2;
	}
	if (len > 0) {
		err_quit("%s: too many bytes (expected %d)\n", argv[5], keybits / 8);
	}
	sadb_add(src->ai_addr, dst->ai_addr, satype, alg, 0x9876, keybits, keydata);
}
