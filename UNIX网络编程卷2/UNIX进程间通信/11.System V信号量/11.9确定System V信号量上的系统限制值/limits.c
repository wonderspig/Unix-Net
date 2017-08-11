#include	"unpipc.h"

	/* 4following are upper limits of values to try */
#define	MAX_NIDS	4096		/* max # semaphore IDs */
#define	MAX_VALUE	1024*1024	/* max semaphore value */
#define	MAX_MEMBERS	4096		/* max # semaphores per semaphore set */
#define	MAX_NOPS	4096		/* max # operations per semop() */
#define	MAX_NPROC	Sysconf(_SC_CHILD_MAX)

int
main(int argc, char **argv)
{
	int		i, j, semid, sid[MAX_NIDS], pipefd[2];
	int		semmni, semvmx, semmsl, semmns, semopn, semaem, semume, semmnu;
	pid_t	*child;
	union semun	arg;
	struct sembuf	ops[MAX_NOPS];

		/* 4see how many sets with one member we can create */
	for (i = 0; i <= MAX_NIDS; i++) {
		sid[i] = semget(IPC_PRIVATE, 1, 644 | IPC_CREAT);
		if (sid[i] == -1) {
			semmni = i;
			printf("%d identifiers open at once\n", semmni);
			break;
		}
	}
		/* 4before deleting, find maximum value using sid[0] */
	for (j = 7; j < MAX_VALUE; j += 8) {
		arg.val = j;
		if (semctl(sid[0], 0, SETVAL, arg) == -1) {
			semvmx = j - 8;
			printf("max semaphore value = %d\n", semvmx);
			break;
		}
	}
	for (j = 0; j < i; j++)
		Semctl(sid[j], 0, IPC_RMID);

		/* 4determine max # semaphores per semaphore set */
	for (i = 1; i <= MAX_MEMBERS; i++) {
		semid = semget(IPC_PRIVATE, i, 644 | IPC_CREAT);
		if (semid == -1) {
			semmsl = i-1;
			printf("max of %d members per set\n", semmsl);
			break;
		}
		Semctl(semid, 0, IPC_RMID);
	}

		/* 4find max of total # of semaphores we can create */
	semmns = 0;
	for (i = 0; i < semmni; i++) {
		sid[i] = semget(IPC_PRIVATE, semmsl, 644 | IPC_CREAT);
		if (sid[i] == -1) {
/* $$.bp$$ */
			/*
			 * Up to this point each set has been created with semmsl
			 * members.  But this just failed, so try recreating this
			 * final set with one fewer member per set, until it works.
			 */
			for (j = semmsl-1; j > 0; j--) {
				sid[i] = semget(IPC_PRIVATE, j, 644 | IPC_CREAT);
				if (sid[i] != -1) {
					semmns += j;
					printf("max of %d semaphores\n", semmns);
					Semctl(sid[i], 0, IPC_RMID);
					goto done;
				}
			}
			err_quit("j reached 0, semmns = %d", semmns);
		}
		semmns += semmsl;
	}
	printf("max of %d semaphores\n", semmns);
done:
	for (j = 0; j < i; j++)
		Semctl(sid[j], 0, IPC_RMID);

		/* 4see how many operations per semop() */
	//此处semmsl没有初始化就继续用这个参数调用Semget,导致直接在包裹函数的错误处理中异常退出
	semid = Semget(IPC_PRIVATE, semmsl, 644 | IPC_CREAT);
	for (i = 1; i <= MAX_NOPS; i++) {
		ops[i-1].sem_num = i-1;
		ops[i-1].sem_op = 1;
		ops[i-1].sem_flg = 0;
		if (semop(semid, ops, i)  == -1) {
			if (errno != E2BIG)
				err_sys("expected E2BIG from semop");
			semopn = i-1;
			printf("max of %d operations per semop()\n", semopn);
			break;
		}
	}
	Semctl(semid, 0, IPC_RMID);

		/* 4determine the max value of semadj */
		/* 4create one set with one semaphore */
	semid = Semget(IPC_PRIVATE, 1, 644 | IPC_CREAT);
	arg.val = semvmx;
	Semctl(semid, 0, SETVAL, arg);		/* set value to max */
	for (i = semvmx-1; i > 0; i--) {
		ops[0].sem_num = 0;
		ops[0].sem_op = -i;
		ops[0].sem_flg = SEM_UNDO;
		if (semop(semid, ops, 1)  != -1) {
			semaem = i;
			printf("max value of adjust-on-exit = %d\n", semaem);
			break;
		}
	}
	Semctl(semid, 0, IPC_RMID);
/* $$.bp$$ */
		/* 4determine max # undo structures */
		/* 4create one set with one semaphore; init to 0 */
	semid = Semget(IPC_PRIVATE, 1, 644 | IPC_CREAT);
	arg.val = 0;
	Semctl(semid, 0, SETVAL, arg);		/* set semaphore value to 0 */
	Pipe(pipefd);
	child = Malloc(MAX_NPROC * sizeof(pid_t));
	for (i = 0; i < MAX_NPROC; i++) {
		if ( (child[i] = fork()) == -1) {
			semmnu = i - 1;
			printf("fork failed, semmnu at least %d\n", semmnu);
			break;
		} else if (child[i] == 0) {
			ops[0].sem_num = 0;			/* child does the semop() */
			ops[0].sem_op = 1;
			ops[0].sem_flg = SEM_UNDO;
			j = semop(semid, ops, 1);	/* 0 if OK, -1 if error */
			Write(pipefd[1], &j, sizeof(j));
			sleep(30);					/* wait to be killed by parent */
			exit(0);					/* just in case */
		}
		/* parent reads result of semop() */
		Read(pipefd[0], &j, sizeof(j));
		if (j == -1) {
			semmnu = i;
			printf("max # undo structures = %d\n", semmnu);
			break;
		}
	}
	Semctl(semid, 0, IPC_RMID);
	for (j = 0; j <= i && child[j] > 0; j++)
		Kill(child[j], SIGINT);

		/* 4determine max # adjust entries per process */
		/* 4create one set with max # of semaphores */
	semid = Semget(IPC_PRIVATE, semmsl, 644 | IPC_CREAT);
	for (i = 0; i < semmsl; i++) {
		arg.val = 0;
		Semctl(semid, i, SETVAL, arg);		/* set semaphore value to 0 */

		ops[i].sem_num = i;
		ops[i].sem_op = 1;					/* add 1 to the value */
		ops[i].sem_flg = SEM_UNDO;
		if (semop(semid, ops, i+1) == -1) {
			semume = i;
			printf("max # undo entries per process = %d\n", semume);
			break;
		}
	}
	Semctl(semid, 0, IPC_RMID);

	exit(0);
}
