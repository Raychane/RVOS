#ifndef __USER_SYSCALL_H__
#define __USER_SYSCALL_H__

#include <stddef.h>
#include <stdarg.h>

// 用户态系统调用接口
int u_task_create(void (*start_routine)(void*), void *param, unsigned char priority, unsigned int timeslice);
void u_task_yield(void);
void u_task_exit(void);
void u_task_delay(unsigned int count);
void* u_malloc(size_t size);
void u_free(void *ptr);
int u_printf(const char *format, ...);
void u_uart_puts(char *s);

#endif /* __USER_SYSCALL_H__ */