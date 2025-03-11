#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "../types.h"

/* 系统调用号 */
#define SYS_task_create  1
#define SYS_task_yield   2
#define SYS_task_exit    3
#define SYS_task_delay   4
#define SYS_malloc       5
#define SYS_free         6
#define SYS_printf       7
#define SYS_uart_puts    8
#define SYS_uart_putc    9   // 新增系统调用号

// 系统调用处理函数声明
int sys_task_create(void (*start_routine)(void *), void *param, uint8_t priority, uint32_t timeslice);
void sys_task_yield(void);
void sys_task_exit(void);
void sys_task_delay(uint32_t count);
void *sys_malloc(size_t size);
void sys_free(void *ptr);
int sys_printf(const char *str);
void sys_uart_puts(char *s);

// 系统调用处理程序
int handle_syscall(reg_t epc, reg_t cause);

#endif /* __SYSCALL_H__ */