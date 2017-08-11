/*
 * Standard I/O wrapper functions.
 */

#include	"unpipc.h"

void
Fclose(FILE *fp)
{
	if (fclose(fp) != 0)
		err_sys("fclose error");
}

FILE *
Fdopen(int fd, const char *type)
{
	FILE	*fp;

	if ( (fp = fdopen(fd, type)) == NULL)
		err_sys("fdopen error");

	return(fp);
}

char *
Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		err_sys("fgets error");

	return (rptr);
}

FILE *
Fopen(const char *pathname, const char *mode)
{
	FILE	*fp;

	if ( (fp = fopen(pathname, mode)) == NULL)
		err_sys("fopen error");

	return(fp);
}

void
Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		err_sys("fputs error");
}

FILE *
Popen(const char *command, const char *mode)
{
	FILE	*fp;

	if ( (fp = popen(command, mode)) == NULL)
		err_sys("popen error");
	return(fp);
}

int
Pclose(FILE *fp)
{
	int		n;

	if ( (n = pclose(fp)) == -1)
		err_sys("pclose error");
	return(n);
}
