#include	"unp.h"

#define	MAXFILES	20
#define	SERV		"80"	/* port number or service name 服务器的端口号 */

struct file {
  char	*f_name;			/* filename 文件名*/
  char	*f_host;			/* hostname or IPv4/IPv6 address 主机名或IP地址 */
  int    f_fd;				/* descriptor 读取文件的套接字描述符*/
  int	 f_flags;			/* F_xxx below 标志(链接,读取,完成)*/
} file[MAXFILES]; //程序最多读MAXFILES个文件

#define	F_CONNECTING	1	/* connect() in progress */
#define	F_READING		2	/* connect() complete; now reading */
#define	F_DONE			4	/* all done */

#define	GET_CMD		"GET %s HTTP/1.0\r\n\r\n"

			/* globals */
int		nconn, nfiles, nlefttoconn, nlefttoread, maxfd;
fd_set	rset, wset;

			/* function prototypes */
void	home_page(const char *, const char *); 
void	start_connect(struct file *);
void	write_get_cmd(struct file *);
