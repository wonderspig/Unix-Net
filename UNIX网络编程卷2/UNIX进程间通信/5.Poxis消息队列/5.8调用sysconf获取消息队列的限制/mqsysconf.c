#include	"unpipc.h"

int
main(int argc, char **argv)
{
	printf("MQ_OPEN_MAX = %ld, MQ_PRIO_MAX = %ld\n",
		   Sysconf(_SC_MQ_OPEN_MAX), Sysconf(_SC_MQ_PRIO_MAX));
	exit(0);
}
