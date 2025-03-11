#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "riscv.h"
#include "platform.h"

#include <stddef.h>
#include <stdarg.h>

/* uart */
extern int uart_putc(char ch);
extern void uart_puts(char *s);

/* printf */
extern int printf(const char *s, ...);
extern void panic(char *s);

/* memory management */
extern void *page_alloc(int npages);
extern void page_free(void *p);

/* task management */
struct context
{
	/* ignore x0 */
	reg_t ra;
	reg_t sp;
	reg_t gp;
	reg_t tp;
	reg_t t0;
	reg_t t1;
	reg_t t2;
	reg_t s0;
	reg_t s1;
	reg_t a0;
	reg_t a1;
	reg_t a2;
	reg_t a3;
	reg_t a4;
	reg_t a5;
	reg_t a6;
	reg_t a7;
	reg_t s2;
	reg_t s3;
	reg_t s4;
	reg_t s5;
	reg_t s6;
	reg_t s7;
	reg_t s8;
	reg_t s9;
	reg_t s10;
	reg_t s11;
	reg_t t3;
	reg_t t4;
	reg_t t5;
	reg_t t6;
	// upon is trap frame

	// save the pc to run in next schedule cycle
	reg_t pc;	   // offset: 31 *4 = 124
	reg_t mstatus; // 新增字段，用于保存 mstatus 寄存器
};

typedef enum
{
	TASK_INVALID,
	TASK_READY,
	TASK_RUNNING,
	TASK_SLEEPING,
	TASK_EXITED
} task_state;

typedef struct
{
	struct context ctx;
	void *param;
	void (*func)(void *param);
	uint8_t priority;
	task_state state;
	uint32_t timeslice;
	uint32_t remaining_timeslice;
} task_t;

#define DEFAULT_TIMESLICE 2

extern int _current;
extern task_t tasks[];

typedef struct timer
{
	void (*func)(void *arg);
	void *arg;
	uint32_t timeout_tick;
	struct timer *next;
} timer;

extern uint32_t get_mtime(void);
extern void print_timers(void);
extern void wake_up_task(void *arg);
extern void just_while(void);
extern void user_task0(void *param);
extern void user_task1(void *param);
extern void user_task(void *param);
extern void print_tasks(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

extern timer *timers, *next_timer;

#define SCHEDULE                         \
	{                                    \
		int id = r_mhartid();            \
		*(uint32_t *)CLINT_MSIP(id) = 1; \
	}

extern void timer_load(int);
void timer_handler();

extern int task_create(void (*start_routin)(void *param), void *param, uint8_t priority, uint32_t timeslice);
extern void task_delay(uint32_t count);
extern void task_yield();
extern void task_exit();

/* scheduler functions */
extern void sched_init(void);
extern void schedule(void);
extern void kernel_scheduler(void);
extern void back_to_os(void);
extern void check_timeslice(void);
extern void *memset(void *, int, size_t);
extern void memory_init(void);
extern void *malloc(size_t);
extern void free(void *);
extern void print_blocks(void);
extern void print_block(void *);

extern int plic_claim(void);
extern void plic_complete(int irq);

extern int spin_lock(void);
extern int spin_unlock(void);

extern timer *timer_create(
	void (*handler)(void *arg),
	void *arg,
	uint32_t timeout);
extern void timer_delete(timer *timer);

extern int _vprintf(const char* s, va_list vl);
extern int _vsprintf(char *out, const char *fmt, va_list vl);

#endif /* __OS_H__ */