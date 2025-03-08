#include "os.h"
#include "syscall.h"

// 系统调用表结构
typedef struct
{
    void *func;      // 函数指针
    int param_count; // 参数数量
} syscall_entry_t;

static syscall_entry_t syscalls[] = {
    [SYS_task_create] = {(void *)task_create, 4},
    [SYS_task_yield] = {(void *)task_yield, 0},
    [SYS_task_exit] = {(void *)task_exit, 0},
    [SYS_task_delay] = {(void *)task_delay, 1},
    [SYS_malloc] = {(void *)malloc, 1},
    [SYS_free] = {(void *)free, 1},
    [SYS_printf] = {(void *)_vprintf, 2},
    [SYS_uart_puts] = {(void *)uart_puts, 1}};

// 处理系统调用
int handle_syscall(reg_t epc, reg_t a0, reg_t a1, reg_t a2, reg_t a3, reg_t a4)
{
    reg_t syscall_num;
    asm volatile("mv %0, a7" : "=r"(syscall_num));

    if (syscall_num > 0 && syscall_num < sizeof(syscalls) / sizeof(syscalls[0]) && syscalls[syscall_num].func)
    {

        void *func = syscalls[syscall_num].func;
        int param_count = syscalls[syscall_num].param_count;

        switch (param_count)
        {
        case 0:
            ((void (*)())func)();
            return 0;
        case 1:
            return (long)((void *(*)(long))func)(a0);
        case 2:
            return ((int (*)(long, long))func)(a0, a1);
        case 3:
            return ((int (*)(long, long, long))func)(a0, a1, a2);
        case 4:
            return ((int (*)(long, long, long, long))func)(a0, a1, a2, a3);
        default:
            return -1;
        }
    }

    printf("未知的系统调用: %d\n", syscall_num);
    return -1;
}