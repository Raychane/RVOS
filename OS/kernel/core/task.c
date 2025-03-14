#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024 * 1024
#define KERNEL_STACK_SIZE 1024 * 1024
#define TASK_USABLE(i) (((tasks[(i)].state) == TASK_READY) || ((tasks[(i)].state) == TASK_RUNNING))
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
int _current = -1;

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

int is_user_mode(void)
{
	reg_t mstatus = r_mstatus();
	// 检查是否在用户模式中(ecall上下文)
	// 如果是ecall处理，当前处于机器模式，但返回特权级是用户模式
	return ((mstatus & MSTATUS_MPP) == 0);
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
	if (tasks[_current].state == TASK_RUNNING)
		tasks[_current].state = TASK_READY;

	// 找到最高优先级
	for (int i = 0; i < _top; i++)
	{
		if (tasks[i].state == TASK_READY && tasks[i].priority < highest_priority)
		{
			highest_priority = tasks[i].priority;
		}
	}

	// 在最高优先级中轮转选择下一个任务
	for (int i = 0; i < _top; i++)
	{
		int idx = (_current + 1 + i) % _top;
		if (tasks[idx].state == TASK_READY && tasks[idx].priority == highest_priority)
		{
			next_task = idx;
			break;
		}
	}

	if (next_task == -1)
	{
		for (int i = 0; i < MAX_TASKS; i++)
		{
			if (tasks[i].state == TASK_READY && tasks[i].priority == highest_priority)
			{
				next_task = i;
				break;
			}
		}
	}

	if (next_task == -1)
	{
		spin_unlock();
		// panic("没有可调度的任务");
		return;
	}

	_current = next_task;
	struct context *next = &(tasks[_current].ctx);

	tasks[_current].state = TASK_RUNNING;
	spin_unlock();
	switch_to(next);
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
int task_create(void (*start_routin)(void *), void *param, uint8_t priority, uint32_t timeslice)
{
	if (_top >= MAX_TASKS) return -1;
	
	// 初始化任务上下文
	tasks[_top].ctx.sp = (reg_t)&task_stack[_top][STACK_SIZE];
	tasks[_top].ctx.pc = (reg_t)start_routin;
	tasks[_top].param = param;
	tasks[_top].func = start_routin;
	tasks[_top].priority = priority;
	tasks[_top].state = TASK_READY;
	tasks[_top].timeslice = timeslice;
	tasks[_top].remaining_timeslice = timeslice;
	
	// 统一设置特权级 - 简化特权级管理
	// 可选：所有任务默认用户态，或根据函数地址确定
	tasks[_top].ctx.mstatus = (0 << 11) | (1 << 7); // 默认用户模式
	
	_top++;
	return _top - 1;
}

/*
 * DESCRIPTION
 *  task_yield() causes the calling task to relinquish the CPU and a new
 *  task gets to run.
 */
void task_yield(void)
{
	spin_lock();
	if (_current != -1 && tasks[_current].state == TASK_RUNNING)
	{
		tasks[_current].state = TASK_READY;
	}
	spin_unlock();

	// 检查当前特权级别并使用对应的调度方式
	if (is_user_mode())
	{
		// 在系统调用上下文中，使用软中断方式触发调度
		SCHEDULE;
	}
	else
	{
		// 在内核态直接调用调度函数
		schedule();
	}
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
		tasks[_current].state = TASK_EXITED;
		uart_puts("任务已退出，并被调度器回收。\n");
	}
	spin_unlock();
	SCHEDULE
	// 如果所有任务都退出，内核可以进入空闲状态或重新启动
	panic("所有任务已退出，系统终止。");
}

// 定时器回调函数，用于唤醒被延迟的任务
void wake_up_task(void *arg)
{
	int task_id = (int)arg;

	// spin_lock();
	if (task_id >= 0 && task_id < MAX_TASKS && tasks[task_id].state == TASK_SLEEPING)
	{
		tasks[task_id].state = TASK_READY;
	}
	// spin_unlock();
}

/*
 * DESCRIPTION
 *  task_delay() causes the calling task to sleep for a specified number of ticks.
 *  - ticks: 延迟的时钟周期数
 */
void task_delay(uint32_t ticks)
{
	spin_lock();
	if (_current == -1)
	{
		spin_unlock();
		return;
	}

	int task_id = _current;
	tasks[task_id].state = TASK_SLEEPING;
	spin_unlock(); // 解锁，因为timer_create可能会阻塞

	// 创建定时器，ticks 后调用 wake_up_task 以唤醒任务
	if (timer_create(wake_up_task, (void *)task_id, ticks) == NULL)
	{
		// 定时器创建失败，恢复任务状态为就绪
		spin_lock();
		tasks[task_id].state = TASK_READY;
		spin_unlock();
	}

	// 让出 CPU，触发调度
	task_yield();
}

/* 获取任务函数名称 */
static const char *get_task_func_name(void (*func)(void *))
{
	if (func == user_task0)
		return "user_task0";
	if (func == user_task1)
		return "user_task1";
	if (func == user_task)
		return "user_task";
	if (func == timer_handler)
		return "timer_handler";
	if (func == task_yield)
		return "task_yield";
	return "unknown";
}

/* 打印任务槽信息的调试函数 */
void print_tasks(void)
{
	printf("\n=== Tasks Debug Info ===\n");

	int active_tasks = 0;
	for (int i = 0; i < MAX_TASKS; i++)
	{
		if (tasks[i].state != TASK_INVALID)
		{
			printf("Task[%d]:\n", i);
			printf("  Function: %s\n", get_task_func_name(tasks[i].func));
			if (tasks[i].func == user_task)
			{
				int task_id = (int)(tasks[i].param);
				printf("  Task ID: %d\n", task_id);
			}
			printf("  Priority: %d\n", tasks[i].priority);

			const char *state_str;
			switch (tasks[i].state)
			{
			case TASK_READY:
				state_str = "READY";
				break;
			case TASK_RUNNING:
				state_str = "RUNNING";
				break;
			case TASK_SLEEPING:
				state_str = "SLEEPING";
				break;
			case TASK_EXITED:
				state_str = "EXITED";
				break;
			default:
				state_str = "UNKNOWN";
				break;
			}
			printf("  State: %s\n", state_str);
			if (i == _current)
			{
				printf("  [CURRENT]\n");
			}
			printf("------------------\n");
			active_tasks++;
		}
	}
	printf("Active tasks: %d, Current: %d\n", active_tasks, _current);
	printf("=== End of Tasks Info ===\n\n");
}