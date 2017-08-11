/* Our own header for the programs that need interface configuration info.
   Include this file, instead of "unp.h". */

#ifndef	__unp_ifi_h
#define	__unp_ifi_h

#include	"unp.h"
#include	<net/if.h>

#define	IFI_NAME	16			/* same as IFNAMSIZ in <net/if.h> 接口名称的长度*/
#define	IFI_HADDR	 8			/* allow for 64-bit EUI-64 in future 硬件地址的长度*/

struct ifi_info {
  char    ifi_name[IFI_NAME];	/* interface name, null-terminated 借口名称,空字符结尾 */
  short   ifi_index;			/* interface index 接口索引 */
  short   ifi_mtu;				/* interface MTU 接口MTU */
  u_char  ifi_haddr[IFI_HADDR];	/* hardware address 硬件地址 */
  u_short ifi_hlen;				/* # bytes in hardware address: 0, 6, 8 硬件地址长度 */
  short   ifi_flags;			/* IFF_xxx constants from <net/if.h> 标志 */
  short   ifi_myflags;			/* our own IFI_xxx flags 我们的标志 */
  struct sockaddr  *ifi_addr;	/* primary address 接口主地址 */
  struct sockaddr  *ifi_brdaddr;/* broadcast address 广播地址 */
  struct sockaddr  *ifi_dstaddr;/* destination address 点到点链路的目的地址 */
  struct ifi_info  *ifi_next;	/* next of these structures 指向下一个结构体 */
};

#define	IFI_ALIAS	1			/* ifi_addr is an alias 此宏代表该接口为别名*/

					/* function prototypes */
struct ifi_info	*get_ifi_info(int, int);
struct ifi_info	*Get_ifi_info(int, int);
void			 free_ifi_info(struct ifi_info *);

#endif	/* __unp_ifi_h */
