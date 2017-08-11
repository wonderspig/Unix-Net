/*
 * Socket wrapper functions.
 * These could all go into separate files, so only the ones needed cause
 * the corresponding function to be added to the executable.  If sockets
 * are a library (SVR4) this might make a difference (?), but if sockets
 * are in the kernel (BSD) it doesn't matter.
 *
 * These wrapper functions also use the same prototypes as POSIX.1g,
 * which might differ from many implementations (i.e., POSIX.1g specifies
 * the fourth argument to getsockopt() as "void *", not "char *").
 *
 * If your system's headers are not correct [i.e., the Solaris 2.5
 * <sys/socket.h> omits the "const" from the second argument to both
 * bind() and connect()], you'll get warnings of the form:
 *warning: passing arg 2 of `bind' discards `const' from pointer target type
 *warning: passing arg 2 of `connect' discards `const' from pointer target type
 */

#include	"unpipc.h"

#include	<stdarg.h>		/* for variable arg lists */

void *
Calloc(size_t n, size_t size)
{
	void	*ptr;

	if ( (ptr = calloc(n, size)) == NULL)
		err_sys("calloc error");
	return(ptr);
}

/* include Close */
void
Close(int fd)
{
	if (close(fd) == -1)
		err_sys("close error");
}
/* end Close */

void
Dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
		err_sys("dup2 error");
}

#ifdef	HAVE_FATTACH
void
Fattach(int fd, const char *path)
{
	if (fattach(fd, path) == -1)
		err_sys("fattach error");
}
#endif

int
Fcntl(int fd, int cmd, void *arg)
{
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
		err_sys("fcntl error");
	return(n);
}

pid_t
Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1)
		err_sys("fork error");
	return(pid);
}

long
Fpathconf(int fd, int name)
{
	long	val;

	errno = 0;		/* in case fpathconf() does not change this */
	if ( (val = fpathconf(fd, name)) == -1) {
		if (errno != 0)
			err_sys("fpathconf error");
		else
			err_sys("fpathconf: %d not defined", name);
	}
	return(val);
}

void
Fstat(int fd, struct stat *ptr)
{
	if (fstat(fd, ptr) == -1)
		err_sys("fstat error");
}

#ifdef	HAVE_SYS_IPC_H
key_t
Ftok(const char *pathname, int id)
{
	key_t	key;

	if ( (key = ftok(pathname, id)) == -1)
		err_sys("ftok error for pathname \"%s\" and id %d", pathname, id);
	return(key);
}
#endif	/* HAVE_SYS_IPC_H */

void
Ftruncate(int fd, off_t length)
{
	if (ftruncate(fd, length) == -1)
		err_sys("ftruncate error");
}

int
Getopt(int argc, char *const *argv, const char *str)
{
	int		opt;

	if ( ( opt = getopt(argc, argv, str)) == '?')
		exit(1);		/* getopt() has already written to stderr */
	return(opt);
}

void
Gettimeofday(struct timeval *tv, void *foo)
{
	if (gettimeofday(tv, foo) == -1)
		err_sys("gettimeofday error");
	return;
}

int
Ioctl(int fd, int request, void *arg)
{
	int		n;

	if ( (n = ioctl(fd, request, arg)) == -1)
		err_sys("ioctl error");
	return(n);	/* streamio of I_LIST returns value */
}

void
Kill(pid_t pid, int signo)
{
	if (kill(pid, signo) == -1)
		err_sys("kill error");
}

off_t
Lseek(int fd, off_t offset, int whence)
{
	off_t	pos;

	if ( (pos = lseek(fd, offset, whence)) == (off_t) -1)
		err_sys("lseek error");
	return(pos);
}

void *
Malloc(size_t size)
{
	void	*ptr;

	if ( (ptr = malloc(size)) == NULL)
		err_sys("malloc error");
	return(ptr);
}

void
Mkfifo(const char *pathname, mode_t mode)
{
	if (mkfifo(pathname, mode) == -1)
		err_sys("mkfifo error for %s", pathname);
}

void
Mktemp(char *template)
{
	if (mkstemp(template) == 0 || template[0] == 0)
		err_quit("mkstemp error");
}

#ifdef	HAVE_SYS_MMAN_H

void *
Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	void	*ptr;

	if ( (ptr = mmap(addr, len, prot, flags, fd, offset)) == MAP_FAILED)
		err_sys("mmap error");
	return(ptr);
}

void
Munmap(void *addr, size_t len)
{
	if (munmap(addr, len) == -1)
		err_sys("munmap error");
}
#endif	/* HAVE_SYS_MMAN_H */

#ifdef	HAVE_MQUEUE_H

mqd_t
Mq_open(const char *pathname, int oflag, ...)
{
	mqd_t	mqd;
	va_list	ap;
	mode_t	mode;
	struct mq_attr	*attr;

	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		attr = va_arg(ap, struct mq_attr *);
		if ( (mqd = mq_open(pathname, oflag, mode, attr)) == (mqd_t) -1)
			err_sys("mq_open error for %s", pathname);
		va_end(ap);
	} else {
		if ( (mqd = mq_open(pathname, oflag)) == (mqd_t) -1)
			err_sys("mq_open error for %s", pathname);
	}
	return(mqd);
}

void
Mq_close(mqd_t mqd)
{
	if (mq_close(mqd) == -1)
		err_sys("mq_close error");
}

void
Mq_unlink(const char *pathname)
{
	if (mq_unlink(pathname) == -1)
		err_sys("mq_unlink error");
}

void
Mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	if (mq_send(mqd, ptr, len, prio) == -1)
		err_sys("mq_send error");
}

ssize_t
Mq_receive(mqd_t mqd, char *ptr, size_t len, unsigned int *prio)
{
	ssize_t	n;

	if ( (n = mq_receive(mqd, ptr, len, prio)) == -1)
		err_sys("mq_receive error");
	return(n);
}

void
Mq_notify(mqd_t mqd, const struct sigevent *notification)
{
	if (mq_notify(mqd, notification) == -1)
		err_sys("mq_notify error");
}

void
Mq_getattr(mqd_t mqd, struct mq_attr *mqstat)
{
	if (mq_getattr(mqd, mqstat) == -1)
		err_sys("mq_getattr error");
}

void
Mq_setattr(mqd_t mqd, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
	if (mq_setattr(mqd, mqstat, omqstat) == -1)
		err_sys("mq_setattr error");
}
#endif	/* HAVE_SYS_MSG_H */

#ifdef	HAVE_SYS_MSG_H
int
Msgget(key_t key, int flag)
{
	int		rc;

	if ( (rc = msgget(key, flag)) == -1)
		err_sys("msgget error");
	return(rc);
}

void
Msgctl(int id, int cmd, struct msqid_ds *buf)
{
	if (msgctl(id, cmd, buf) == -1)
		err_sys("msgctl error");
}

void
Msgsnd(int id, const void *ptr, size_t len, int flag)
{
	if (msgsnd(id, ptr, len, flag) == -1)
		err_sys("msgsnd error");
}

ssize_t
Msgrcv(int id, void *ptr, size_t len, int type, int flag)
{
	ssize_t	rc;

	if ( (rc = msgrcv(id, ptr, len, type, flag)) == -1)
		err_sys("msgrcv error");
	return(rc);
}
#endif	/* HAVE_SYS_MSG_H */

int
Open(const char *pathname, int oflag, ...)
{
	int		fd;
	va_list	ap;
	mode_t	mode;

	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		if ( (fd = open(pathname, oflag, mode)) == -1)
			err_sys("open error for %s", pathname);
		va_end(ap);
	} else {
		if ( (fd = open(pathname, oflag)) == -1)
			err_sys("open error for %s", pathname);
	}
	return(fd);
}

long
Pathconf(const char *pathname, int name)
{
	long	val;

	errno = 0;		/* in case pathconf() does not change this */
	if ( (val = pathconf(pathname, name)) == -1) {
		if (errno != 0)
			err_sys("pathconf error");
		else
			err_sys("pathconf: %d not defined", name);
	}
	return(val);
}

void
Pipe(int *fds)
{
	if (pipe(fds) < 0)
		err_sys("pipe error");
}

ssize_t
Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1)
		err_sys("read error");
	return(n);
}

/* include Select */
int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
	int		n;

again:
	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0) {
		if (errno == EINTR)
			goto again;
		else
			err_sys("select error");
	} else if (n == 0 && timeout == NULL)
		err_quit("select returned 0 with no timeout");
	return(n);		/* can return 0 on timeout */
}
/* end Select */

#ifdef	HAVE_SEMAPHORE_H

sem_t *
Sem_open(const char *pathname, int oflag, ...)
{
	sem_t	*sem;
	va_list	ap;
	mode_t	mode;
	unsigned int	value;

	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		value = va_arg(ap, unsigned int);
		if ( (sem = sem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("sem_open error for %s", pathname);
		va_end(ap);
	} else {
		if ( (sem = sem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("sem_open error for %s", pathname);
	}
	return(sem);
}

void
Sem_close(sem_t *sem)
{
	if (sem_close(sem) == -1)
		err_sys("sem_close error");
}

void
Sem_unlink(const char *pathname)
{
	if (sem_unlink(pathname) == -1)
		err_sys("sem_unlink error");
}

void
Sem_init(sem_t *sem, int pshared, unsigned int value)
{
	if (sem_init(sem, pshared, value) == -1)
		err_sys("sem_init error");
}

void
Sem_destroy(sem_t *sem)
{
	if (sem_destroy(sem) == -1)
		err_sys("sem_destroy error");
}

void
Sem_wait(sem_t *sem)
{
	if (sem_wait(sem) == -1)
		err_sys("sem_wait error");
}

int
Sem_trywait(sem_t *sem)
{
	int		rc;

	if ( (rc = sem_trywait(sem)) == -1 && errno != EAGAIN)
		err_sys("sem_trywait error");
	return(rc);
}

/* include Sem_post */
void
Sem_post(sem_t *sem)
{
	if (sem_post(sem) == -1)
		err_sys("sem_post error");
}
/* end Sem_post */

void
Sem_getvalue(sem_t *sem, int *valp)
{
	if (sem_getvalue(sem, valp) == -1)
		err_sys("sem_getvalue error");
}

#endif	/* HAVE_SEMAPHORE_H */

#ifdef	HAVE_SYS_SEM_H
int
Semget(key_t key, int nsems, int flag)
{
	int		rc;

	if ( (rc = semget(key, nsems, flag)) == -1)
		err_sys("semget error");
	return(rc);
}

void
Semop(int id, struct sembuf *opsptr, size_t nops)
{
	if (semop(id, opsptr, nops) == -1)
		err_sys("semctl error");
}

int
Semctl(int id, int semnum, int cmd, ...)
{
	int		rc;
	va_list	ap;
	union semun	arg;

	if (cmd == GETALL || cmd == SETALL || cmd == SETVAL ||
		cmd == IPC_STAT || cmd == IPC_SET) {
		va_start(ap, cmd);		/* init ap to final named argument */
		arg = va_arg(ap, union semun);
		if ( (rc = semctl(id, semnum, cmd, arg)) == -1)
			err_sys("semctl error");
		va_end(ap);
	} else {
		if ( (rc = semctl(id, semnum, cmd)) == -1)
			err_sys("semctl error");
	}
	return(rc);
}

#endif	/* HAVE_SYS_SEM_H */

#ifdef	HAVE_SHM_OPEN_PROTO

int
Shm_open(const char *pathname, int oflag, mode_t mode)
{
	int		fd;

	if ( (fd = shm_open(pathname, oflag, mode)) == -1)
		err_sys("shm_open error for %s", pathname);
	return(fd);
}


void
Shm_unlink(const char *pathname)
{
	if (shm_unlink(pathname) == -1)
		err_sys("shm_unlink error");
}

#endif	/* HAVE_SHM_OPEN_PROTO */

#ifdef	HAVE_SYS_SHM_H

int
Shmget(key_t key, size_t size, int flags)
{
	int		rc;

	if ( (rc = shmget(key, size, flags)) == -1)
		err_sys("shmget error");
	return(rc);
}

void *
Shmat(int id, const void *shmaddr, int flags)
{
	void	*ptr;

	if ( (ptr = shmat(id, shmaddr, flags)) == (void *) -1)
		err_sys("shmat error");
	return(ptr);
}

void
Shmdt(const void *shmaddr)
{
	if (shmdt(shmaddr) == -1)
		err_sys("shmdt error");
}

void
Shmctl(int id, int cmd, struct shmid_ds *buff)
{
	if (shmctl(id, cmd, buff) == -1)
		err_sys("shmctl error");
}

#endif	/* HAVE_SYS_SHM_H */

void
Sigaddset(sigset_t *set, int signo)
{
	if (sigaddset(set, signo) == -1)
		err_sys("sigaddset error");
}

void
Sigdelset(sigset_t *set, int signo)
{
	if (sigdelset(set, signo) == -1)
		err_sys("sigdelset error");
}

void
Sigemptyset(sigset_t *set)
{
	if (sigemptyset(set) == -1)
		err_sys("sigemptyset error");
}

void
Sigfillset(sigset_t *set)
{
	if (sigfillset(set) == -1)
		err_sys("sigfillset error");
}

int
Sigismember(const sigset_t *set, int signo)
{
	int		n;

	if ( (n = sigismember(set, signo)) == -1)
		err_sys("sigismember error");
	return(n);
}

void
Sigpending(sigset_t *set)
{
	if (sigpending(set) == -1)
		err_sys("sigpending error");
}

void
Sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	if (sigprocmask(how, set, oset) == -1)
		err_sys("sigprocmask error");
}

#ifdef	HAVE_SIGINFO_T_STRUCT
void
Sigqueue(pid_t pid, int signo, const union sigval val)
{
	if (sigqueue(pid, signo, val) == -1)
		err_sys("sigqueue error");
}
#endif

#ifdef	HAVE_SIGWAIT
void
Sigwait(const sigset_t *set, int *signo)
{
	int		n;

	if ( (n = sigwait(set, signo)) == 0)
		return;
	errno = n;
	err_sys("sigwait error");
}
#endif

void
Stat(const char *pathname, struct stat *ptr)
{
	if (stat(pathname, ptr) == -1)
		err_sys("stat error");
}

char *
Strdup(const char *str)
{
	char	*ptr;

	if ( (ptr = strdup(str)) == NULL)
		err_sys("strdup error");
	return(ptr);
}

long
Sysconf(int name)
{
	long	val;

	errno = 0;		/* in case sysconf() does not change this */
	if ( (val = sysconf(name)) == -1) {
		if (errno != 0)
			err_sys("sysconf error");
		else
			err_sys("sysconf: %d not defined", name);
	}
	return(val);
}

#ifdef	HAVE_SYS_SYSCTL_H

#include	<sys/param.h>
#include	<sys/sysctl.h>

void
Sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
	   void *newp, size_t newlen)
{
	if (sysctl(name, namelen, oldp, oldlenp, newp, newlen) == -1)
		err_sys("sysctl error");
}
#endif

void
Unlink(const char *pathname)
{
	if (unlink(pathname) == -1)
		err_sys("unlink error for %s", pathname);
}

void *
Valloc(size_t size)
{
	void	*ptr;

#ifdef	HAVE_VALLOC
	if ( (ptr = valloc(size)) == NULL)
		err_sys("valloc error");
#else
	if ( (ptr = malloc(size)) == NULL)
		err_sys("malloc error (Valloc)");
#endif
	return(ptr);
}

pid_t
Wait(int *iptr)
{
	pid_t	pid;

	if ( (pid = wait(iptr)) == -1)
		err_sys("wait error");
	return(pid);
}

pid_t
Waitpid(pid_t pid, int *iptr, int options)
{
	pid_t	retpid;

	if ( (retpid = waitpid(pid, iptr, options)) == -1)
		err_sys("waitpid error");
	return(retpid);
}

void
Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes)
		err_sys("write error");
}
