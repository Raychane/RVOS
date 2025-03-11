#include "include/user/user_syscall.h"
#include "syscall.h"
#include "os.h"

// 用户态系统调用包装函数
int u_task_create(void (*start_routine)(void*), void *param, unsigned char priority, unsigned int timeslice)
{
    int ret;
    asm volatile(
        "li a7, %1\n"
        "mv a0, %2\n"
        "mv a1, %3\n"
        "mv a2, %4\n"
        "mv a3, %5\n"
        "ecall\n"
        "mv %0, a0"
        : "=r"(ret)
        : "i"(SYS_task_create), "r"(start_routine), "r"(param), "r"(priority), "r"(timeslice)
        : "a0", "a1", "a2", "a3", "a7");
    return ret;
}

void u_task_yield(void)
{
    asm volatile(
        "li a7, %0\n"
        "ecall"
        :
        : "i"(SYS_task_yield)
        : "a7");
}

void u_task_exit(void)
{
    asm volatile(
        "li a7, %0\n"
        "ecall"
        :
        : "i"(SYS_task_exit)
        : "a7");
}

void u_task_delay(unsigned int count)
{
    asm volatile(
        "li a7, %1\n"
        "mv a0, %0\n"
        "ecall"
        :
        : "r"(count), "i"(SYS_task_delay)
        : "a0", "a7");
}

void* u_malloc(size_t size)
{
    void* ret;
    asm volatile(
        "li a7, %1\n"
        "mv a0, %2\n"
        "ecall\n"
        "mv %0, a0"
        : "=r"(ret)
        : "i"(SYS_malloc), "r"(size)
        : "a0", "a7");
    return ret;
}

void u_free(void *ptr)
{
    asm volatile(
        "li a7, %1\n"
        "mv a0, %0\n"
        "ecall"
        :
        : "r"(ptr), "i"(SYS_free)
        : "a0", "a7");
}

// 替换现有的 u_printf 实现
int u_printf(const char *format, ...)
{
    // 首先在用户空间格式化字符串
    char buffer[256];  // 预先分配足够的缓冲区
    va_list args;
    va_start(args, format);
    
    // 在用户空间完成格式化
    int len = _vsprintf(buffer, format, args);
    va_end(args);
    
    // 然后调用系统调用只传递格式化后的字符串
    int ret;
    asm volatile(
        "li a7, %1\n"
        "mv a0, %2\n"
        "ecall\n"
        "mv %0, a0"
        : "=r"(ret)
        : "i"(SYS_uart_puts), "r"(buffer)  // 使用uart_puts替代printf
        : "a0", "a7");
    
    return len;
}

// 添加一个简单的vsprintf实现或从printf.c借用

void u_uart_puts(char *s)
{
    asm volatile(
        "li a7, %1\n"
        "mv a0, %0\n"
        "ecall"
        :
        : "r"(s), "i"(SYS_uart_puts)
        : "a0", "a7");
}