#ifndef __EPOLL_H__
#define __EPOLL_H__

typedef struct SChannel event;

//创建epoll对象，size默认为1024，他表示epoll中红黑树的容量，仅作为一个参考
/**
.epoll_create的参数在linux2.6内核之后就没有什么作用了，这个参数会被忽略，但是必须要大于0。
*/
int createEPoll();
int freeEPoll(int fd);

void addEvent(int fd, event* ev);
void delEvent(int fd, event* ev);
void modifyEvent(int fd,event* ev);

struct timeval dispatchEvent(int fd,int timeout);



#endif