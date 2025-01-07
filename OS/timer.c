#include "os.h"

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

//static struct timer *timer_list = NULL; // 链表头指针
static uint32_t _tick = 0;

static skiplist timer_list;

static unsigned int next = 1;

/* 设置随机数种子 */
void srand(unsigned int seed) {
    next = seed;
}

/* 生成随机数 */
int rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

static int random_level() {
    int level = 1;
    while (rand() % 2 && level < MAX_LEVEL) {
        level++;
    }
    return level;
}

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
    /* each CPU has a separate source of timer interrupts. */
    int id = r_mhartid();
    
    *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init() {
    timer_list.header = (struct timer *)malloc(sizeof(struct timer));
    for (int i = 0; i < MAX_LEVEL; i++) {
        timer_list.header->forward[i] = NULL;
    }
    timer_list.level = 1;
    timer_load(TIMER_INTERVAL);
    w_mie(r_mie() | MIE_MTIE);
}

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout) {
    if (NULL == handler || 0 == timeout) {
        return NULL;
    }

    struct timer *new_timer = (struct timer *)malloc(sizeof(struct timer));
    if (NULL == new_timer) {
        printf("malloc failed for new_timer\n");
        return NULL;
    }

    new_timer->func = handler;
    new_timer->arg = arg;
    new_timer->timeout_tick = _tick + timeout;

    int level = random_level();
    struct timer *update[MAX_LEVEL];
    struct timer *x = timer_list.header;

    for (int i = timer_list.level - 1; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i]->timeout_tick < new_timer->timeout_tick) {
            x = x->forward[i];
        }
        update[i] = x;
    }

    for (int i = 0; i < level; i++) {
        new_timer->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_timer;
    }

    if (level > timer_list.level) {
        timer_list.level = level;
    }
    printf("Timer created: %p, timeout: %d, level: %d\n", new_timer, new_timer->timeout_tick, level);
    return new_timer;
}

void timer_delete(struct timer *timer) {
    if (timer == NULL) {
        return;
    }

    struct timer *update[MAX_LEVEL];
    struct timer *x = timer_list.header;

    for (int i = timer_list.level - 1; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i] != timer) {
            x = x->forward[i];
        }
        update[i] = x;
    }

    for (int i = 0; i < timer_list.level; i++) {
        if (update[i]->forward[i] == timer) {
            update[i]->forward[i] = timer->forward[i];
        }
    }
    printf("Timer deleted: %p\n", timer);

    free(timer);
}

/* this routine should be called in interrupt context (interrupt is disabled) */
void timer_check() {
    struct timer *x = timer_list.header->forward[0];
    while (x != NULL && _tick >= x->timeout_tick) {
        struct timer *expired_timer = x;
        x = x->forward[0];
        expired_timer->func(expired_timer->arg);
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