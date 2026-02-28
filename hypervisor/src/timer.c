#include "timer.h"
#include "types.h"
#include "arch.h"

static uint64_t boot_count;
static uint32_t counter_freq;

void timer_init(void)
{
    // read_sysreg(counter_freq, cntfrq_el0);
    // read_sysreg(boot_count, cntpct_el0);
    asm volatile("mrs %0, cntfrq_el0" : "=r"(counter_freq));
    asm volatile("mrs %0, cntpct_el0" : "=r"(boot_count));
}

uint64_t timer_get_us(void)
{
    uint64_t now;
    // read_sysreg(now, cntpct_el0);
    asm volatile("mrs %0, cntpct_el0" : "=r"(now));
    uint64_t elapsed = now - boot_count;
    return (elapsed * 1000000UL) / counter_freq;
}
