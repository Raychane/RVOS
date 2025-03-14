#include "include/user/user_syscall.h"
#include "syscall.h"
#include "os.h"
#include "kernel/syscall/syscall_wrappers.h"

// 统一的系统调用包装函数 - mini-riscv-os风格
static long syscall(long num, long arg0, long arg1, long arg2, long arg3) 
{
    long ret;
    asm volatile (
        "mv a7, %1\n"
        "mv a0, %2\n"
        "mv a1, %3\n" 
        "mv a2, %4\n"
        "mv a3, %5\n"
        "ecall\n"
        "mv %0, a0"
        : "=r" (ret)
        : "r" (num), "r" (arg0), "r" (arg1), "r" (arg2), "r" (arg3)
        : "a0", "a1", "a2", "a3", "a7"
    );
    return ret;
}

// 基于统一系统调用修改所有现有函数
int u_task_create(void (*start_routine)(void *), void *param, unsigned char priority, unsigned int timeslice)
{
    return syscall(SYS_task_create, (long)start_routine, (long)param, priority, timeslice);
}

void u_task_yield(void)
{
    syscall(SYS_task_yield, 0, 0, 0, 0);
}

void u_task_exit(void)
{
    syscall(SYS_task_exit, 0, 0, 0, 0); 
}

void u_task_delay(unsigned int count)
{
    syscall(SYS_task_delay, count, 0, 0, 0);
}

void *u_malloc(size_t size)
{
    return (void*)syscall(SYS_malloc, size, 0, 0, 0);
}

void u_free(void *ptr)
{
    syscall(SYS_free, (long)ptr, 0, 0, 0);
}

int u_printf(const char *format, ...)
{
    // 首先在用户空间格式化字符串
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = _vsprintf(buffer, format, args);
    va_end(args);
    
    // 然后通过系统调用发送字符串
    syscall(SYS_uart_puts, (long)buffer, 0, 0, 0);
    return len;
}

void u_uart_puts(char *s)
{
    syscall(SYS_uart_puts, (long)s, 0, 0, 0);
}

// 任务管理相关包装函数
reg_t sys_task_create_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    return task_create((void (*)(void*))p0, (void*)p1, (uint8_t)p2, (uint32_t)p3);
}

reg_t sys_task_yield_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    task_yield();
    return 0;
}

reg_t sys_task_exit_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    task_exit();
    return 0;  // 实际上不会返回
}

reg_t sys_task_delay_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    task_delay((uint32_t)p0);
    return 0;
}

// 内存管理相关包装函数
reg_t sys_malloc_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    return (reg_t)malloc((size_t)p0);
}

reg_t sys_free_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    free((void*)p0);
    return 0;
}

// 输入输出相关包装函数
reg_t sys_printf_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    uart_puts((char*)p0);  // 简单版本，直接调用uart_puts
    return 0;
}

reg_t sys_uart_puts_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    uart_puts((char*)p0);
    return 0;
}

reg_t sys_uart_putc_wrapper(reg_t p0, reg_t p1, reg_t p2, reg_t p3) {
    uart_putc((char)p0);
    return 0;
}