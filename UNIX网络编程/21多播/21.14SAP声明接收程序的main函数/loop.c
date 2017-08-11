#include	"mysdr.h"

void
loop(int sockfd, socklen_t salen)
{
	socklen_t		len;
	ssize_t			n;
	char			*p;
	struct sockaddr	*sa;
	struct sap_packet {
	  uint32_t	sap_header;
	  uint32_t	sap_src;
	  char		sap_data[BUFFSIZE];
	} buf;

	sa = Malloc(salen); //动态分配一个地址空间

	for ( ; ; ) {
		len = salen; //把存放地址结构体的大小复制一份
		n = Recvfrom(sockfd, &buf, sizeof(buf) - 1, 0, sa, &len); //从加入多播的套接字中读取数据,存放在buf中
		((char *)&buf)[n] = 0;			/* null terminate 缓冲区空字符结尾*/
		buf.sap_header = ntohl(buf.sap_header); //32位SAP首部,转换成本机的字节序
		//显示来自sa地址的SAP散列值
		//Sock_ntop函数:协议无关的把二进制地址转换成表达式格式
		printf("From %s hash 0x%04x\n", Sock_ntop(sa, len),
				buf.sap_header & SAP_HASH_MASK);
		//确认SAP首部是否为我们处理的类型
		//不处理在首部中使用IP6分组地址,压缩的或者加密的分组
		if (((buf.sap_header & SAP_VERSION_MASK) >> SAP_VERSION_SHIFT) > 1) {
			err_msg("... version field not 1 (0x%08x)", buf.sap_header);
			continue;
		}
		if (buf.sap_header & SAP_IPV6) {
			err_msg("... IPv6");
			continue;
		}
		if (buf.sap_header & (SAP_DELETE|SAP_ENCRYPTED|SAP_COMPRESSED)) {
			err_msg("... can't parse this packet type (0x%08x)", buf.sap_header);
			continue;
		}
		//跳过可能存在任何认证数据和分组内容类型
		p = buf.sap_data + ((buf.sap_header & SAP_AUTHLEN_MASK)
							>> SAP_AUTHLEN_SHIFT);
		if (strcmp(p, "application/sdp") == 0)
			p += 16;
		printf("%s\n", p); //显示分组内容
	}
}
