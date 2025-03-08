#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024 * 1024
#define KERNEL_STACK_SIZE 1024 * 1024
#define DEFAULT_TIMESLICE 10

/*
 * In the standard RISC-V calling convention, the stack pointer sp
 * is always 16-byte aligned.
 */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
uint8_t __attribute__((aligned(16))) kernel_stack_kernel[KERNEL_STACK_SIZE];
task_t tasks[MAX_TASKS];
struct context kernel_ctx;

/*
 * _top is used to mark the max available position of tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
static int _current = -1;

void kernel_scheduler()
{
	while (1)
	{
		SCHEDULE
	}
}

// 在 `sched_init` 中创建内核调度任务
void sched_init()
{
	w_mscratch((reg_t)&kernel_ctx);
	w_mie(r_mie() | MIE_MSIE);

	// 初始化内核调度任务上下文
	kernel_ctx.sp = (reg_t)&kernel_stack_kernel[KERNEL_STACK_SIZE];
	kernel_ctx.pc = (reg_t)kernel_scheduler;

	// task_create(kernel_scheduler, NULL, 0); // 优先级最高的内核任务

	// 其他初始化代码...
}
void back_to_os(void)
{
	switch_to(&kernel_ctx);
}

/*
 * implement a priority-based scheduler
 */
void schedule()
{
	spin_lock();
	if (_top <= 0)
	{
		spin_unlock();
		return;
	}

	int next_task = -1;
	uint8_t highest_priority = 255;

	// 找到最高优先级
	for (int i = 0; i < _top; i++)
	{
		if (tasks[i].valid && tasks[i].priority < highest_priority)
		{
			highest_priority = tasks[i].priority;
		}
	}

	// 在最高优先级中轮转选择下一个任务
	for (int i = 0; i < _top; i++)
	{
		int idx = (_current + 1 + i) % _top;
		if (tasks[idx].valid && tasks[idx].priority == highest_priority)
		{
			next_task = idx;
			break;
		}
	}

	if (next_task == -1)
	{
		for (int i = 0; i < MAX_TASKS; i++)
		{
			if (tasks[i].priority == highest_priority)
			{
				next_task = i;
				break;
			}
		}
	}

	if (next_task == -1)
	{
		panic("没有可调度的任务");
		spin_unlock();
		return;
	}

	_current = next_task;
	struct context *next = &(tasks[_current].ctx);

	switch_to(next);
	spin_unlock();
}

void check_timeslice()
{
	tasks[_current].remaining_timeslice--;
	if (tasks[_current].remaining_timeslice == 0)
	{
		tasks[_current].remaining_timeslice = tasks[_current].timeslice;
		task_yield();
	}
}

/*
 * DESCRIPTION
 *  Create a task.
 *  - start_routin: task routine entry
 *  - param: parameter to pass to the task
 *  - priority: priority of the task
 * RETURN VALUE
 *  0: success
 *  -1: if error occurred
 */
int task_create(void (*start_routin)(void *param), void *param, uint8_t priority)
{
	spin_lock();
	if (_top >= MAX_TASKS)
	{
		spin_unlock();
		return -1;
	}

	tasks[_top].ctx.sp = (reg_t)&task_stack[_top][STACK_SIZE] & ~0xF;
	tasks[_top].ctx.pc = (reg_t)start_routin;
	tasks[_top].ctx.a0 = param;
	// 初始化 mstatus 为机器模式
	tasks[_top].ctx.mstatus = (3 << 11) | (1 << 7); // MPP = 3 (机器模式), MPIE = 1

	// 其他初始化代码...
	tasks[_top].priority = priority;
	tasks[_top].valid = 1;
	tasks[_top].timeslice = DEFAULT_TIMESLICE;
	tasks[_top].remaining_timeslice = DEFAULT_TIMESLICE;

	printf("创建任务: %p\n", (void *)tasks[_top].ctx.pc);

	_top++;

	spin_unlock();
	return 0;
}

/*
 * DESCRIPTION
 *  task_yield() causes the calling task to relinquish the CPU and a new
 *  task gets to run.
 */
void task_yield()
{
	// 触发一个软件中断，内核调度任务将负责实际的任务切换
	int hart = r_mhartid();
	*(uint32_t *)CLINT_MSIP(hart) = 1;
}
/*
 * DESCRIPTION
 *  task_exit() causes the calling task to exit and be removed from the scheduler.
 */
void task_exit()
{
	spin_lock();
	if (_current != -1)
	{
		tasks[_current].valid = 0;
		uart_puts("任务已退出，并被调度器回收。\n");
	}
	spin_unlock();
	SCHEDULE
	// 如果所有任务都退出，内核可以进入空闲状态或重新启动
	panic("所有任务已退出，系统终止。");
}
/*
 * a very rough implementation, just to consume the cpu
 */
void task_delay(volatile int count)
{
	spin_lock();
	count *= 50000;
	while (count--)
		;
	spin_unlock();
}
