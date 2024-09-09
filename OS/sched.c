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

void sched_init()
{
	w_mscratch((reg_t)&kernel_ctx);

	// w_mscratch(0);

	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);

	// Initialize kernel scheduler task context 是将ms寄存器的地址写入ctx，因此不需要初始化，自有寄存器帮我写内容
	kernel_ctx.sp = (reg_t)&kernel_stack[KERNEL_STACK_SIZE];
	kernel_ctx.pc = (reg_t)kernel_scheduler;
}

void back_to_os(void)
{
	switch_to(&kernel_ctx);
}

/*
 * implement a priority-based scheduler
 */
void schedule() {
	spin_lock();
	if (_top <= 0) {
		spin_unlock();
		return;
	}

	int next_task = -1;
	uint8_t highest_priority = 255;

	for (int i = 0; i < _top; i++) {
		if (tasks[i].state == TASK_READY && tasks[i].priority < highest_priority) {
			highest_priority = tasks[i].priority;
			next_task = i;
		}
	}

	if (next_task == -1) {
		spin_unlock();
		return;
	}

	_current = next_task;
	tasks[_current].state = TASK_RUNNING; // 设置当前任务为运行状态
	struct context *next = &(tasks[_current].ctx);

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
int task_create(void (*start_routin)(void *param), void *param, uint8_t priority, uint32_t timeslice)
{
	spin_lock();
	if (_top >= MAX_TASKS)
	{
		spin_unlock();
		return -1;
	}

	tasks[_top].ctx.sp = (reg_t)&task_stack[_top][STACK_SIZE];
	tasks[_top].ctx.pc = (reg_t)start_routin;
	tasks[_top].ctx.a0 = param;
	tasks[_top].priority = priority;
	tasks[_top].valid = 1;
	tasks[_top].timeslice = timeslice;
	tasks[_top].remaining_timeslice = timeslice;
    tasks[_top].state = TASK_READY;

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
	schedule();
	// back_to_os();
	/* trigger a machine-level software interrupt */
	// int id = r_mhartid();
	//*(uint32_t*)CLINT_MSIP(id) = 1;
}

/*
 * DESCRIPTION
 *  task_exit() causes the calling task to exit and be removed from the scheduler.
 */
void task_exit() {
	if (_current < 0 || _current >= MAX_TASKS) {
		return;
	}

	tasks[_current].state = TASK_EXITED;
	schedule();
}

static void task_wakeup(void *arg) {
    int task_id = (int)arg;
    tasks[task_id].state = TASK_READY;
}

int task_delay(uint32_t tick) {
	if (_current < 0 || _current >= MAX_TASKS) {
		return -1;
	}

	tasks[_current].state = TASK_SLEEPING;
	timer_create(task_wakeup, (void *)_current, tick);

	schedule();

	return 0;
}

void kernel_scheduler()
{
	while (1)
	{
		uart_puts("kernel_scheduler check\n");
		// Kernel scheduler task does nothing, just yields to the next task
		schedule();
	}
}
