#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

static uint32_t _tick = 0;

#define MAX_TIMER 10
static struct timer *timer_list = NULL;

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
    /* each CPU has a separate source of timer interrupts. */
    int id = r_mhartid();
    
    *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init()
{
    struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		t->func = NULL; /* use .func to flag if the item is used */
		t->arg = NULL;
		t++;
	}

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

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout) {
    if (handler == NULL || timeout == 0) {
        return NULL;
    }

    struct timer *new_timer = (struct timer *)malloc(sizeof(struct timer));
    if (new_timer == NULL) {
        return NULL;
    }

    new_timer->func = handler;
    new_timer->arg = arg;
    new_timer->timeout_tick = _tick + timeout;
    new_timer->next = NULL;
    new_timer->prev = NULL;

    spin_lock();

    if (timer_list == NULL) {
        timer_list = new_timer;
    } else {
        struct timer *current = timer_list;
        while (current->next != NULL && current->next->timeout_tick < new_timer->timeout_tick) {
            current = current->next;
        }
        new_timer->next = current->next;
        if (current->next != NULL) {
            current->next->prev = new_timer;
        }
        current->next = new_timer;
        new_timer->prev = current;
    }

    spin_unlock();

    return new_timer;
}

void timer_delete(struct timer *timer) {
    if (timer == NULL) {
        return;
    }

    spin_lock();

    if (timer->prev != NULL) {
        timer->prev->next = timer->next;
    } else {
        timer_list = timer->next;
    }
    if (timer->next != NULL) {
        timer->next->prev = timer->prev;
    }

    spin_unlock();

    free(timer);
}

static void timer_check() {
    struct timer *current = timer_list;
    while (current != NULL && current->timeout_tick <= _tick) {
        current->func(current->arg);
        struct timer *expired_timer = current;
        current = current->next;
        timer_delete(expired_timer);
    }
}

void timer_handler() {
    _tick++;
    printf("tick: %d\n", _tick);

    timer_check();

    timer_load(TIMER_INTERVAL);

    schedule();
}