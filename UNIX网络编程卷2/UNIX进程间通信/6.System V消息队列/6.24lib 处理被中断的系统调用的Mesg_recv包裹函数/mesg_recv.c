/* include mesg_recv */
#include	"mesg.h"

ssize_t
mesg_recv(int id, struct mymesg *mptr)
{
	ssize_t	n;

	n = msgrcv(id, &(mptr->mesg_type), MAXMESGDATA, mptr->mesg_type, 0);
	mptr->mesg_len = n;		/* return #bytes of data */

	return(n);				/* -1 on error, 0 at EOF, else >0 */
}
/* end mesg_recv */

/* include Mesg_recv */
ssize_t
Mesg_recv(int id, struct mymesg *mptr)
{
	ssize_t	n;

	do {
		n = mesg_recv(id, mptr);
	} while (n == -1 && errno == EINTR); //如果被信号打断,则重新调用mesg_recv

	if (n == -1)
		err_sys("mesg_recv error");

	return(n);
}
/* end Mesg_recv */
