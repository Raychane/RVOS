#include "os.h"

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */
extern void uart_init(void);
extern void page_init(void);
extern void sched_init(void);
extern void schedule(void);
extern void os_main(void);
extern void trap_init(void);
extern void plic_init(void);
extern void timer_init(void);

void start_kernel(void)
{
    uart_init();
    uart_puts("Hello, RVOS!\n");

    page_init();

    trap_init();

    plic_init();

    timer_init();

    sched_init();

    memory_init(); // 初始化内存管理

    os_main();

    while (1)
    {
        SCHEDULE
    }
    uart_puts("Would not go here!\n");
    while (1)
    {
    }; // stop here!
}
