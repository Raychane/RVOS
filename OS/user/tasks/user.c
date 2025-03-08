#include "os.h"

#define DELAY 1000

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

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
    // 创建内核调度任务已经在 `sched_init` 中完成
    // 创建用户任务
    task_create(user_task0, NULL, 128); // 优先级 1
    task_create(user_task1, NULL, 128); // 优先级 2
    task_create(user_task, (void *)2, 3); // 优先级 3
    task_create(user_task, (void *)3, 3); // 优先级 3
}
