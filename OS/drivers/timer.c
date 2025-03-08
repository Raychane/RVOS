#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

static uint32_t _tick = 0;

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
    /* each CPU has a separate source of timer interrupts. */
    int id = r_mhartid();
    
    *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init()
{
    /*
     * On reset, mtime is cleared to zero, but the mtimecmp registers 
     * are not reset. So we have to init the mtimecmp manually.
     */
    timer_load(TIMER_INTERVAL);

    /* enable machine-mode timer interrupts. */
    w_mie(r_mie() | MIE_MTIE);

    /* enable machine-mode global interrupts. */
    // w_mstatus(r_mstatus() | MSTATUS_MIE);
}


void timer_handler() 
{
<<<<<<< Updated upstream:OS/timer.c
	printf("tick: %d\n", _tick);
	timer_load(TIMER_INTERVAL);
    task_yield();
    check_timeslice();
=======
    // 确保地址正确对齐
    volatile uint32_t *mtime_ptr = (volatile uint32_t *)(CLINT_BASE + 0xBFF8);
    return *mtime_ptr;
}

timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout)
{
    timer *t = malloc(sizeof(timer));
    if (t == NULL)
    {
        return NULL;
    }
    t->func = handler;
    t->arg = arg;
    t->timeout_tick = get_mtime() + timeout * TIMER_INTERVAL;
    t->next = NULL;
    timers = insert_to_timer_list(timers, t);
    // timer_load(timeout); // 确保加载定时器
    return t;
}

void timer_delete(timer *timer)
{
    timers = delete_from_timer_list(timers, timer);
    free(timer);
}

void run_timer_list()
{
    //printf("timer expired: %ld\n", timers->timeout_tick);
    //printf("current tick: %ld\n", get_mtime());
    while (timers != NULL && timers->timeout_tick <= get_mtime())
    {
        timer *expired = timers;
        timers = timers->next;
        
        // 执行定时器回调
        expired->func(expired->arg);
        
        // 释放定时器
        free(expired);
    }
    if (timers == NULL)
    {
        timer_create(schedule, NULL, 1);
        spin_unlock();
        return;
    }
    timer_load(timers->timeout_tick);
}

void timer_handler()
{
    spin_lock();
    //printf("tick: %d\n", _tick++);
    //printf("mtime: %d\n", get_mtime());
    //printf("mtimecmp: %d\n", get_mtimecmp());
    //print_tasks();
    //print_timers();
    // if (timers->func == timer_handler)
    // {
    //     timer_create(timer_handler, NULL, 1);
    // }
    run_timer_list();
    spin_unlock();
    // check_timeslice();
}

/* 打印定时器链表信息的调试函数 */
void print_timers(void)
{
    printf("\n=== Timer List Debug Info ===\n");
    printf("MTIMECMP:%d\n", get_mtimecmp());
    if (timers == NULL)
    {
        printf("Timer list is empty\n");
        return;
    }

    timer *current = timers;
    int count = 0;

    while (current != NULL)
    {
        printf("Timer[%d]:\n", count++);
        printf("  timeout_tick: %d\n", current->timeout_tick);
        const char *func_name = "unknown";
        if (current->func == timer_handler)
        {
            func_name = "timer_handler";
        }
        else if (current->func == task_yield)
        {
            func_name = "task_yield";
        }
        else if (current->func == wake_up_task)
        {
            func_name = "wake_up_task";
        }
        else if (current->func == schedule)
        {
            func_name = "schedule";
        }

        // ... 添加其他你需要识别的函数

        printf("  func name: %s\n", func_name);
        printf("  arg: %p\n", current->arg);
        printf("  next: %p\n", (void *)current->next);

        current = current->next;
    }
    printf("=== End of Timer List ===\n\n");
>>>>>>> Stashed changes:OS/drivers/timer.c
}