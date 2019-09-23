#ifndef __CHANNEL_H__
#define __CHANNEL_H__

typedef struct SChannel event;
typedef struct TcpServer serverManager;

//C函数方式实现回调
typedef void(*event_callback_func)(int fd, event*e, void*arg);

struct SChannel{
	int fd;
	int eventFlag;
	int activeEvent;
	struct timeval time;
	//可读事件回调函数
	event_callback_func eventReadHandler;
	void *readClientData;
	event_callback_func eventWriteHandler;
	void *writeClientData;

	int isWorking;
	int epollFd;
};


event* eventCreate(int fd, short event_flag, event_callback_func read_cb,
	void* r_arg, event_callback_func write_cb, void* w_arg);

int eventStart(event* ev);
void eventStop(event* ev);
void eventFree(event* ev);

void eventAddIoLoop(int epoll_fd, event* ev);
void eventEnableWriting(event* ev);
void eventDisableRriting(event* ev);

void eventModifyFlag(event*ev,int addFlag,bool add);
void eventHandler(event* ev);

#endif