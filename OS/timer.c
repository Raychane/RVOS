#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

//static struct timer *timer_list = NULL; // 链表头指针
static struct timer timer_list[10];
static int timer_count = 0;

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

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout)
{
    if (NULL == handler || 0 == timeout) {
        return NULL;
    }

    // struct timer *new_timer = (struct timer *)malloc(sizeof(struct timer));
    // if (NULL == new_timer) {
    //     return NULL;
    // }

    // new_timer->func = handler;
    // new_timer->arg = arg;
    // new_timer->timeout_tick = _tick + timeout;
    // new_timer->next = NULL;

    // spin_lock();

    // if (timer_list == NULL || timer_list->timeout_tick > new_timer->timeout_tick) {
    //     new_timer->next = timer_list;
    //     timer_list = new_timer;
    // } else {
    //     struct timer *current = timer_list;
    //     while (current->next != NULL && current->next->timeout_tick <= new_timer->timeout_tick) {
    //         current = current->next;
    //     }
    //     new_timer->next = current->next;
    //     current->next = new_timer;
    // }

    // spin_unlock();

    struct timer *new_timer = &timer_list[timer_count++];
    new_timer->func = handler;
    new_timer->arg = arg;
    new_timer->timeout_tick = _tick + timeout;

    return new_timer;
}

void timer_delete(struct timer *timer)
{
    if (timer == NULL) {
        return;
    }

    // spin_lock();

    // if (timer_list == timer) {
    //     timer_list = timer_list->next;
    // } else {
    //     struct timer *current = timer_list;
    //     while (current->next != NULL && current->next != timer) {
    //         current = current->next;
    //     }
    //     if (current->next == timer) {
    //         current->next = timer->next;
    //     }
    // }

    // spin_unlock();

    // free(timer);

        for (int i = 0; i < timer_count; i++) {
        if (&timer_list[i] == timer) {
            for (int j = i; j < timer_count - 1; j++) {
                timer_list[j] = timer_list[j + 1];
            }
            timer_count--;
            break;
        }
    }
}

/* this routine should be called in interrupt context (interrupt is disabled) */
static inline void timer_check()
{
    // spin_lock();

    // while (timer_list != NULL && _tick >= timer_list->timeout_tick) {
    //     struct timer *expired_timer = timer_list;
    //     timer_list = timer_list->next;

    //     spin_unlock();
    //     expired_timer->func(expired_timer->arg);
    //     spin_lock();

    //     free(expired_timer);
    // }

    // spin_unlock();

    for (int i = 0; i < timer_count; i++) {
        if (_tick >= timer_list[i].timeout_tick) {
            struct timer *expired_timer = &timer_list[i];
            expired_timer->func(expired_timer->arg);
            timer_delete(expired_timer);
            i--; // adjust index after deletion
        }
    }    
}

void timer_handler() 
{
    _tick++;
    printf("tick: %d\n", _tick);
    timer_check();
    timer_load(TIMER_INTERVAL);
    schedule();
}
