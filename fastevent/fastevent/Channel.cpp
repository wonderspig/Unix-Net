#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "Channel.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Epoll.h"




event* eventCreate(int fd, short event_flag, event_callback_func read_cb,
	void* r_arg, event_callback_func write_cb, void* w_arg){
	event*ev = (event*)malloc(sizeof(event));
	if (ev == NULL){
		debug_ret("file: %s, line: %d", __FILE__, __LINE__);
		return NULL;
	}
	ev->fd = fd;
	ev->eventFlag = event_flag;
	ev->activeEvent = 0;
	ev->eventReadHandler = read_cb;
	ev->readClientData = r_arg;
	ev->eventWriteHandler = write_cb;
	ev->writeClientData = w_arg;
	return ev;
}

int eventStart(event* ev){

}
void eventStop(event* ev){
	if (ev->isWorking == 0){
		return;
	}
	delEvent(ev->epollFd,ev);
	//避免重复删除
	ev->isWorking = 0;
}
void eventFree(event* ev){
	eventStop(ev);
	close(ev->fd);
	free(ev);
}

void eventAddIoLoop(int epoll_fd, event* ev){
	addEvent(epoll_fd, ev);
	ev->epollFd = epoll_fd;
	ev->isWorking = 1;
}
//添加或者删除
void eventModifyFlag(event*ev, int addFlag, bool add){
	int flag = ev->eventFlag | addFlag;
	if (!add){
		flag = ev->eventFlag &= ~flag;
	}
	if (ev->isWorking == 0)  {
		debug_msg("epoll_fd cannot modify, because it's not working now! %s, line: %d", __FILE__, __LINE__);
		return;
	}
	ev->eventFlag = flag;
	modifyEvent(ev->epollFd, ev);
}
void eventEnableWriting(event* ev){
	eventModifyFlag(ev,EPOLLOUT,true);
}
void eventDisableRriting(event* ev){
	eventModifyFlag(ev, EPOLLOUT, false);
}

void eventHandler(event* ev){
	if (ev->activeEvent&EPOLLRDHUP){

	}
	if (ev->activeEvent&(EPOLLIN | EPOLLPRI)){
		if (ev->eventReadHandler){
			//执行回调函数
			ev->eventReadHandler(ev->fd, ev, ev->readClientData);
		}
	}
	if (ev->activeEvent&(EPOLLOUT )){
		if (ev->eventWriteHandler){
			//执行回调函数
			ev->eventWriteHandler(ev->fd, ev, ev->writeClientData);
		}
	}
}