#define _GNU_SOURCE

/* config.h.  Generated automatically by configure.  */
/* Define the following if you have the corresponding header */
#define	CPU_VENDOR_OS "i686-pc-linux-gnu"

/* *INDENT-OFF* */
/* #undef	HAVE_DOOR_H */				/* <door.h> */
#define	HAVE_MQUEUE_H 1				/* <mqueue.h> */
#define	HAVE_POLL_H 1				/* <poll.h> */
#define	HAVE_PTHREAD_H 1			/* <pthread.h> */
#define	HAVE_RPC_RPC_H 1			/* <rpc/rpc.h> */
#define	HAVE_SEMAPHORE_H 1			/* <semaphore.h> */
#define	HAVE_STRINGS_H 1			/* <strings.h> */
/* #undef	HAVE_SYS_FILIO_H */			/* <sys/filio.h> */
#define	HAVE_SYS_IOCTL_H 1			/* <sys/ioctl.h> */
#define	HAVE_SYS_IPC_H 1			/* <sys/ipc.h> */
#define	HAVE_SYS_MMAN_H 1			/* <sys/mman.h> */
#define	HAVE_SYS_MSG_H 1			/* <sys/msg.h> */
#define	HAVE_SYS_SEM_H 1			/* <sys/sem.h> */
#define	HAVE_SYS_SHM_H 1			/* <sys/shm.h> */
#define	HAVE_SYS_SELECT_H 1			/* <sys/select.h> */
#define	HAVE_SYS_SYSCTL_H 1	/* <sys/sysctl.h> */
#define	HAVE_SYS_TIME_H 1			/* <sys/time.h> */

/* Define if we can include <time.h> with <sys/time.h> */
#define	TIME_WITH_SYS_TIME 1

/* Define the following if the function is provided */
#define	HAVE_BZERO 1
/* #undef	HAVE_FATTACH */
#define	HAVE_POLL 1
#define	HAVE_PSELECT 1
#define	HAVE_SIGWAIT 1
#define	HAVE_VALLOC 1
#define	HAVE_VSNPRINTF 1

/* Define the following if the function prototype is in a header */
#define	HAVE_GETHOSTNAME_PROTO 1	/* <unistd.h> */
#define	HAVE_GETRUSAGE_PROTO 1		/* <sys/resource.h> */
#define	HAVE_PSELECT_PROTO 1	/* <sys/select.h> */
#define	HAVE_SHM_OPEN_PROTO 1		/* <sys/mman.h> */
#define	HAVE_SNPRINTF_PROTO 1		/* <stdio.h> */
/* #undef	HAVE_THR_SETCONCURRENCY_PROTO */		/* <thread.h> */

/* Define the following if the structure is defined. */
#define	HAVE_SIGINFO_T_STRUCT 1		/* <signal.h> */
#define	HAVE_TIMESPEC_STRUCT 1		/* <time.h> */
/* #undef	HAVE_SEMUN_UNION */		/* <sys/sem.h> */

/* Devices */
#define	HAVE_DEV_ZERO 1

/* Define the following to the appropriate datatype, if necessary */
/* #undef	int8_t */				/* <sys/types.h> */
/* #undef	int16_t */				/* <sys/types.h> */
/* #undef	int32_t */				/* <sys/types.h> */
//#define	uint8_t unsigned char				/* <sys/types.h> */
//#define	uint16_t unsigned short				/* <sys/types.h> */
//#define	uint32_t unsigned int				/* <sys/types.h> */
/* #undef	size_t */				/* <sys/types.h> */
/* #undef	ssize_t */				/* <sys/types.h> */

/* #undef	POSIX_IPC_PREFIX */
#define	RPCGEN_ANSIC 1			/* defined if rpcgen groks -C option */
/* *INDENT-ON* */

// add by jcq
typedef unsigned long ulong_t;
#define MSG_R 0400
#define MSG_W 0200