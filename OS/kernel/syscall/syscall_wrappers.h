#ifndef SYSCALL_WRAPPERS_H
#define SYSCALL_WRAPPERS_H

#include "os.h"

// 统一的系统调用包装函数签名
typedef reg_t (*syscall_wrapper_t)(reg_t, reg_t, reg_t, reg_t);

// 任务管理相关包装函数
reg_t sys_task_create_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_task_yield_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_task_exit_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_task_delay_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);

// 内存管理相关包装函数
reg_t sys_malloc_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_free_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);

// 输入输出相关包装函数
reg_t sys_printf_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_uart_puts_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);
reg_t sys_uart_putc_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3);

#endif