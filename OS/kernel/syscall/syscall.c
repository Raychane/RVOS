#include "os.h"
#include "syscall.h"
#include "syscall_wrappers.h"

// 系统调用表 - 指向包装函数
syscall_wrapper_t syscall_table[] = {
    [0] NULL, // 保留
    [SYS_task_create] sys_task_create_wrapper,
    [SYS_task_yield] sys_task_yield_wrapper,
    [SYS_task_exit] sys_task_exit_wrapper,
    [SYS_task_delay] sys_task_delay_wrapper,
    [SYS_malloc] sys_malloc_wrapper,
    [SYS_free] sys_free_wrapper,
    [SYS_printf] sys_printf_wrapper,
    [SYS_uart_puts] sys_uart_puts_wrapper,
    [SYS_uart_putc] sys_uart_putc_wrapper,
    // 可以添加更多系统调用
};

// 处理系统调用
int handle_syscall(reg_t epc, reg_t cause)
{
    if (_current < 0) return -1;
    
    // 获取系统调用号和参数
    reg_t syscall_num = tasks[_current].ctx.a7;
    
    if (syscall_num > 0 && syscall_num < sizeof(syscall_table)/sizeof(syscall_table[0]) && 
        syscall_table[syscall_num] != NULL) {
        
        // 直接调用包装函数
        reg_t ret = syscall_table[syscall_num](
            tasks[_current].ctx.a0,
            tasks[_current].ctx.a1,
            tasks[_current].ctx.a2,
            tasks[_current].ctx.a3
        );
        
        // 将返回值存入a0
        tasks[_current].ctx.a0 = ret;
        
        // 可选调试输出
        printf("系统调用: num=%d\n", syscall_num);
        
        return 0;
    }
    
    return -1;
}