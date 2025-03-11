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
    [SYS_printf] = {(void *)uart_puts, 1},
    [SYS_uart_puts] = {(void *)uart_puts, 1},
    [SYS_uart_putc] = {(void *)uart_putc, 1} // 新增系统调用
};

// 处理系统调用
int handle_syscall(reg_t epc, reg_t cause)
{
    // 确认当前有正在运行的任务
    if (_current < 0) {
        printf("无活动任务执行系统调用\n");
        return -1;
    }
    
    // 从已保存的上下文中获取真正的系统调用参数
    reg_t syscall_num = tasks[_current].ctx.a7;  // 系统调用号
    reg_t a0 = tasks[_current].ctx.a0;
    reg_t a1 = tasks[_current].ctx.a1;
    reg_t a2 = tasks[_current].ctx.a2;
    reg_t a3 = tasks[_current].ctx.a3;
    reg_t a4 = tasks[_current].ctx.a4;
    
    // 调试输出
    printf("系统调用: num=%d, a0=%p\n", syscall_num, (void*)a0);
    
    if (syscall_num > 0 && syscall_num < sizeof(syscalls)/sizeof(syscalls[0]) && syscalls[syscall_num].func)
    {
        void *func = syscalls[syscall_num].func;
        int param_count = syscalls[syscall_num].param_count;
        
        // 原有的函数调用逻辑不变
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