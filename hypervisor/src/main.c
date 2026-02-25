#include "layout.h"
#include "printf.h"
#include "hyper_log.h"
#include "spinlock.h"
#include "malloc.h"
#include "kalloc.h"
#include "gicv3.h"

__attribute__((aligned(SZ_4K))) char sp_stack[SZ_4K * NCPU] = {0};

static void print_logo(void)
{
    LOG_INFO("=====> Welcome to Mini-Hyper OS! <=====\n");
    printf("\n");
}

int hyper_init_primary()
{
    print_logo();

    /* xmalloc init */
    malloc_init();
    /* kalloc init */
    kalloc_init();

    /* gicv3 init */
    gic_v3_init();
    irq_enable;

    return 0;
}
