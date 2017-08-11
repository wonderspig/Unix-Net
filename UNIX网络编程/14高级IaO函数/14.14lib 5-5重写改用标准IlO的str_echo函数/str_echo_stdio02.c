#include	"unp.h"

void
str_echo(int sockfd)
{
	char		line[MAXLINE];
	FILE		*fpin, *fpout;

	fpin = Fdopen(sockfd, "r"); //把sockfd转换成流(读)
	fpout = Fdopen(sockfd, "w"); //把sockfd转换成流(写)

	while (Fgets(line, MAXLINE, fpin) != NULL) //从fpin流中读,存放在line中
		Fputs(line, fpout); //把line写到流fpout中
}
