#include	"unpipc.h"

#define	MAX_NIDS	4096

int
main(int argc, char **argv)
/* $$.bp$$ */
{
	int		i, j, shmid[MAX_NIDS];
	void	*addr[MAX_NIDS];
	unsigned long	size;

		/* 4see how many identifiers we can "open" */
	for (i = 0; i <= MAX_NIDS; i++) {
		shmid[i] = shmget(IPC_PRIVATE, 1024, SVSHM_MODE | IPC_CREAT);
		if (shmid[i] == -1) {
			printf("%d identifiers open at once\n", i);
			break;
		}
	}
	for (j = 0; j < i; j++)
		Shmctl(shmid[j], IPC_RMID, NULL);

		/* 4now see how many we can "attach" */
	//由于循环使用了MAX_NIDS,所以会在Shmget函数中崩溃
	for (i = 0; i <= MAX_NIDS; i++) {
		shmid[i] = Shmget(IPC_PRIVATE, 1024, SVSHM_MODE | IPC_CREAT);
		addr[i] = shmat(shmid[i], NULL, 0);
		if (addr[i] == (void *) -1) {
			printf("%d shared memory segments attached at once\n", i);
			Shmctl(shmid[i], IPC_RMID, NULL);	/* the one that failed */
			break;
		}
	}
	for (j = 0; j < i; j++) {
		Shmdt(addr[j]);
		Shmctl(shmid[j], IPC_RMID, NULL);
	}

		/* 4see how small a shared memory segment we can create */
	for (size = 1; ; size++) {
		shmid[0] = shmget(IPC_PRIVATE, size, SVSHM_MODE | IPC_CREAT);
		if (shmid[0] != -1) {		/* stop on first success */
			printf("minimum size of shared memory segment = %lu\n", size);
			Shmctl(shmid[0], IPC_RMID, NULL);
			break;
		}
	}

		/* 4see how large a shared memory segment we can create */
	for (size = 65536; ; size += 4096) {
		shmid[0] = shmget(IPC_PRIVATE, size, SVSHM_MODE | IPC_CREAT);
		if (shmid[0] == -1) {		/* stop on first failure */
			printf("maximum size of shared memory segment = %lu\n", size-4096);
			break;
		}
		Shmctl(shmid[0], IPC_RMID, NULL);
	}

	exit(0);
}
