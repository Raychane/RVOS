#include "os.h"
#include "include/user/user_syscall.h"

#define DELAY 1

void just_while(void)
{
	while (1)
	{
		// for(int i=0;i<=100000000;i++);
		// uart_putc('a');
		;
	}
}

void user_task0(void *param)
{
	uart_puts("Task 0: Created!\n");
	while (1)
	{
		uart_puts("Task 0: Running...\n");
		task_delay(DELAY);
	}
}

void user_task1(void *param)
{
	uart_puts("Task 1: Created!\n");
	while (1)
	{
		uart_puts("Task 1: Running...\n");
		task_delay(DELAY);
	}
}

void user_task(void *param)
{
	int task_id = (int)param;
	printf("Task %d: Created!\n", task_id);
	int iter_cnt = task_id;
	while (1)
	{
		printf("Task %d: Running...\n", task_id);
		task_delay(DELAY);
		if (iter_cnt-- == 0)
		{
			break;
		}
	}
	printf("Task %d: Finished!\n", task_id);
	task_exit();
}

void user_syscall_task(void *param)
{
    int task_id = (int)param;
    u_uart_puts("User Syscall Task: Started!\n");
    
    // 使用系统调用接口打印
    u_printf("Task %d: 使用系统调用接口\n", task_id);
    
    for (int i = 0; i < 3; i++) {
		u_printf("Task %d: 迭代 %d\n", task_id, i);
        u_task_delay(1); // 使用系统调用接口延迟
    }
    
    // 分配内存测试
    void *mem = u_malloc(64);
    if (mem != NULL) {
        u_printf("Task %d: 内存分配成功: %p\n", task_id, mem);
        u_free(mem); // 释放内存
    }
    
    u_uart_puts("User Syscall Task: 完成!\n");
    u_task_exit(); // 使用系统调用接口退出
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
    // 直接使用内核API创建所有任务
    task_create(just_while, NULL, 129, DEFAULT_TIMESLICE);
    task_create(user_task0, NULL, 128, DEFAULT_TIMESLICE);
    task_create(user_task1, NULL, 128, DEFAULT_TIMESLICE);
    task_create(user_syscall_task, (void*)2, 3, DEFAULT_TIMESLICE);
}