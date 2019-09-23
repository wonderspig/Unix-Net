#ifndef __LOGGER_H__
#define __LOGGER_H__

void debug_ret(const char *fmt, ...);
void debug_sys(const char *fmt, ...);
void debug_msg(const char *fmt, ...);
void debug_quit(const char *fmt, ...);

#endif