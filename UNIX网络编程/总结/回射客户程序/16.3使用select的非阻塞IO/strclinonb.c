/* include nonb1 */
#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, val, stdineof;
	ssize_t		n, nwritten;
	fd_set		rset, wset;
	char		to[MAXLINE], fr[MAXLINE];
	char		*toiptr, *tooptr, *friptr, *froptr;

	val = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK); //设置连接到服务器的套接字为非阻塞

	val = Fcntl(STDIN_FILENO, F_GETFL, 0);
	Fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK); //设置标准输入为非阻塞

	val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
	Fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK); //设置标准输出为非阻塞

	/* initialize buffer pointers 初始化缓冲区指针 */
	toiptr = tooptr = to; //toiptr从标准输入读取数据的存放处,tooptr为下一个要写到套接字的字节
	friptr = froptr = fr; //friptr从套接字读取数据的存放处,froptr为下一个要写到标准输出的字节
	stdineof = 0; //标志,初始化为还没有收到eof
	//select的参数maxfdp1,取最大的描述符+1
	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
	for ( ; ; ) { 
		FD_ZERO(&rset); //清空描述符集rset
		FD_ZERO(&wset); //清空描述符集wset
		if (stdineof == 0 && toiptr < &to[MAXLINE]) //如果还没有收到eof,且指针toiptr还没有走到&to[MAXLINE]处(还能从标准输入读入的字节数)
			FD_SET(STDIN_FILENO, &rset);	/* read from stdin 打开读描述符集中标准输入所在的位(打算传给select关心标准输入的可读状态) */
		if (friptr < &fr[MAXLINE]) //指针friptr还没有走到&fr[MAXLINE]处(还能从套接字中接受数据)
			FD_SET(sockfd, &rset);			/* read from socket 打开读描述符集中sockfd所在的位(打算传给select关心socket的可读状态)*/
		if (tooptr != toiptr) //指针tooptr还没有走到toiptr(还有数据需要写向套接字)
			FD_SET(sockfd, &wset);			/* data to write to socket 打开写描述符集中sockfd所在的位(打算传给select关心socket的可写状态) */
		if (froptr != friptr) //指针froptr还没有走到friptr(还有数据需要写向标准输出)
			FD_SET(STDOUT_FILENO, &wset);	/* data to write to stdout 打开写描述符集中标准输出所在的位(打算传给select关心标准输出的可写状态) */

		Select(maxfdp1, &rset, &wset, NULL, NULL); //描述符集设置完毕,调用select
/* end nonb1 */
/* include nonb2 */
		//在select返回之后,进行判断,哪些描述符准备好了
		if (FD_ISSET(STDIN_FILENO, &rset)) { //如果是读描述符集中的标准输入准备好了
			//那么从标准输入读,从指针toiptr处位置开始存放读到的内容,最多读&to[MAXLINE] - toiptr个字节
			if ( (n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) { 
				if (errno != EWOULDBLOCK) //read出错不是因为没有数据,而是其他原因
					err_sys("read error on stdin");

			} else if (n == 0) { //read读到eof
#ifdef	VOL2
				fprintf(stderr, "%s: EOF on stdin\n", gf_time());
#endif
				stdineof = 1;			/* all done with stdin 把收到eof的标志置为1 */
				if (tooptr == toiptr) //如果指针tooptr已经走到了toiptr处(代表已经把所有的数据都写向了套接字)
					Shutdown(sockfd, SHUT_WR);/* send FIN 关闭写端sockfd(当读完数据之后会引发tcp终止序列)*/

			} else { //read正常读取
#ifdef	VOL2
				fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
#endif
				toiptr += n;			/* # just read 读了多少个字节,指针toiptr就往后走多少格 */
				FD_SET(sockfd, &wset);	/* try and write to socket below 
										   已经有数据从标准输入读进来了,下面需要把这些数据往sockfd中写,自然要打开写描述符集中的socket位
										   以便调用select时关心socket的写状态 */
			}
		}

		if (FD_ISSET(sockfd, &rset)) { //如果时读描述符集中的socket准备好了
			//从socket中读,从指针friptr处位置开始存放读到的内容,最多读&fr[MAXLINE] - friptr个字节
			if ( (n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) { 
				if (errno != EWOULDBLOCK) //read出错不是因为没有数据,而是其他原因
					err_sys("read error on socket");

			} else if (n == 0) { //read读到了eof
#ifdef	VOL2
				fprintf(stderr, "%s: EOF on socket\n", gf_time());
#endif
				//如果stdineof=1,说明我们已经在标准输入中输入过eof,sockfd的写端已经关闭,此时返回主函数,主函数会退出，造成正常的tcp终止序列执行
				if (stdineof) 
					return;		/* normal termination */
				else //我们没有在标准输入中发出过eof,服务端进程意外终止
					err_quit("str_cli: server terminated prematurely");

			} else { //read正常读取到数据
#ifdef	VOL2
				fprintf(stderr, "%s: read %d bytes from socket\n",
								gf_time(), n);
#endif
				friptr += n;		/* # just read 读了多少个字节,就把friptr指针往后走多少格 */
				FD_SET(STDOUT_FILENO, &wset);	/* try and write below 
												   已经有数据socket读进来了,下面需要把这些数据往标准输出中写,自然要打开写描述符集中的标注输出位
										   		   以便调用select时关心标准输出的写状态 */
			}
		}
/* end nonb2 */
/* include nonb3 */
		//如果时标准输出的写准备好了,并且根据n的值判断,是否时刚刚从读sockfd的分支中走出来
		if (FD_ISSET(STDOUT_FILENO, &wset) && ( (n = friptr - froptr) > 0)) {
			//确实是刚刚从读sockfd的分支中走出来
			if ( (nwritten = write(STDOUT_FILENO, froptr, n)) < 0) { //从指针froptr处开始把数据写入标准输出
				if (errno != EWOULDBLOCK) //read不是因为标准输出缓冲区已满而写不进去,而是因为其他错误
					err_sys("write error to stdout");

			} else { //此分支代表正常写入
#ifdef	VOL2
				fprintf(stderr, "%s: wrote %d bytes to stdout\n",
								gf_time(), nwritten);
#endif
				froptr += nwritten;		/* # just written 写入多少个字节,则指针froptr就往后走多少格 */
				if (froptr == friptr) //如果指针froptr走到了指针friptr处(说明所有从套接字中读取的数据,已经全部写入了标准输出)
					froptr = friptr = fr;	/* back to beginning of buffer 则所有的指针都返回缓冲区的起始处 */
			}
		}

		//如果时sockfd的写准备好了,并且根据n的值判断,是否时刚刚从读标准输入的分支中走出来
		if (FD_ISSET(sockfd, &wset) && ( (n = toiptr - tooptr) > 0)) {
			//确实是刚刚从读标准输入的分支中走出来
			if ( (nwritten = write(sockfd, tooptr, n)) < 0) { //从指针froptr处开始把数据写入sockfd
				if (errno != EWOULDBLOCK) //read不是因为标准输出缓冲区已满而写不进去,而是因为其他错误
					err_sys("write error to socket");

			} else { //此分支代表正常写入
#ifdef	VOL2
				fprintf(stderr, "%s: wrote %d bytes to socket\n",
								gf_time(), nwritten);
#endif
				tooptr += nwritten;	/* # just written 写入多少个字节,则指针tooptr就往后走多少格 */
				if (tooptr == toiptr) { //如果指针tooptr走到了指针toiptr处(说明所有从标准输入中读取的数据,已经全部写入了sockfd)
					toiptr = tooptr = to;	/* back to beginning of buffer  则所有的指针都返回缓冲区的起始处 */
					if (stdineof) //判断有没有收到标准输入的eof
						Shutdown(sockfd, SHUT_WR);	/* send FIN 关闭写端sockfd(当读完数据之后会引发tcp终止序列) */
				}
			}
		}
	}
}
/* end nonb3 */
