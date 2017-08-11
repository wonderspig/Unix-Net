#include	"unp.h"

int
connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	int				flags, n, error;
	socklen_t		len;
	fd_set			rset, wset;
	struct timeval	tval;

	flags = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); //设置套接字为非阻塞

	error = 0; //清空error的值
	if ( (n = connect(sockfd, saptr, salen)) < 0) //调用connect链接服务器
		if (errno != EINPROGRESS) //如果不是非阻塞的连接超时,则出错返回-1,否则忽略EINPROGRESS错误
			return(-1);

	/* Do whatever we want while the connect is taking place. 
	   我们可以让connect在三路握手的过程中做些别的事情 */

	if (n == 0) //connect连接成功
		goto done;	/* connect completed immediately */

	FD_ZERO(&rset); //清空读描述符集
	FD_SET(sockfd, &rset); //打开sockfd所对应的位
	wset = rset; //把写描述符集设置成和读描述符集一样
	tval.tv_sec = nsec; //设置时间(传参进来的秒数填入tval结构体)
	tval.tv_usec = 0; //微秒数设置为0

	//调用select在读写方面关系sockfd
	if ( (n = Select(sockfd+1, &rset, &wset, NULL,
					 nsec ? &tval : NULL)) == 0) { 
		close(sockfd);		/* timeout 链接超时 */
		errno = ETIMEDOUT; //设置error的值
		return(-1); //链接失败返回-1
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) { //如果sockfd即可读又可写,则代表有错误
		len = sizeof(error); //计算一下error值的大小
		//获得错误状态填入error中,如果error为0则代表链接成功建立,否则出错
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) 
			return(-1);			/* Solaris pending error */
	} else //否则就是select错误
		err_quit("select error: sockfd not set");

done:
	Fcntl(sockfd, F_SETFL, flags);	/* restore file status flags 把sockfd的设置还原*/

	if (error) { //如果有错误
		close(sockfd);		/* just in case 关闭套接字*/
		errno = error; //设置错误值
		return(-1); //返回-1
	}
	return(0);
}
