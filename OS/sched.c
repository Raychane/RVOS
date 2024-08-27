#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024
#define KERNEL_STACK_SIZE 1024

/*
 * In the standard RISC-V calling convention, the stack pointer sp
 * is always 16-byte aligned.
 */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
uint8_t __attribute__((aligned(16))) kernel_stack[KERNEL_STACK_SIZE];
task_t tasks[MAX_TASKS];
struct context kernel_ctx;

/*
 * _top is used to mark the max available position of tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
static int _current = -1;

static void w_mscratch(reg_t x)
{
	asm volatile("csrw mscratch, %0" : : "r"(x));
}

void sched_init()
{
	w_mscratch((reg_t)&kernel_ctx);

	// Initialize kernel scheduler task context 是将ms寄存器的地址写入ctx，因此不需要初始化，自有寄存器帮我写内容
	//kernel_ctx.sp = (reg_t)&kernel_stack[KERNEL_STACK_SIZE];
	//kernel_ctx.ra = (reg_t)kernel_scheduler;
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
	if (_top <= 0)
	{
		panic("Num of task should be greater than zero!");
		return;
	}

	int next_task = -1;
	uint8_t highest_priority = 255;

	// Find the highest priority
	for (int i = 0; i < _top; i++)
	{
		if (tasks[i].valid && tasks[i].priority < highest_priority)
		{
			highest_priority = tasks[i].priority;
		}
	}

	// Find the next task with the same highest priority
	for (int i = 0; i < _top; i++)
	{
		if (tasks[i].valid && tasks[i].priority == highest_priority)
		{
			if (i > _current)
			{
				next_task = i;
				break;
			}
		}
	}

	if (next_task == -1)
	{
		for (int i = 0; i < MAX_TASKS; i++)
		{
			if (tasks[i].valid && tasks[i].priority == highest_priority)
			{
				next_task = i;
				break;
			}
		}
	}
	if (next_task == -1)
	{
		panic("no schedulable task");
		return;
	}

	//if (next_task == _current)
	{
		//return;这段代码会导致函数返回到kernel中并继续反复调用调度函数，造成死循环
	}

	_current = next_task;
	struct context *next = &(tasks[_current].ctx);

	// Switch to kernel scheduler task first
	// switch_to(&kernel_ctx);//?

	// Kernel scheduler task will switch to the next task
	switch_to(next);
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
	if (_top < MAX_TASKS)
	{
		tasks[_top].ctx.sp = (reg_t)&task_stack[_top][STACK_SIZE];
		tasks[_top].ctx.ra = (reg_t)start_routin;
		tasks[_top].ctx.a0 = (reg_t)param;
		tasks[_top].priority = priority;
		tasks[_top].valid = 1;
		_top++;
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * DESCRIPTION
 *  task_yield() causes the calling task to relinquish the CPU and a new
 *  task gets to run.
 */
void task_yield()
{
	back_to_os();
}

/*
 * DESCRIPTION
 *  task_exit() causes the calling task to exit and be removed from the scheduler.
 */
void task_exit()
{
	if (_current != -1)
	{
		tasks[_current].valid = 0;
		back_to_os();
	}
}

/*
 * a very rough implementation, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--)
		;
}

void kernel_scheduler()
{
	while (1)
	{
		// Kernel scheduler task does nothing, just yields to the next task
		task_yield();
	}
}