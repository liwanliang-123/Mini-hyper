#include "layout.h"

__attribute__((aligned(SZ_4K))) char sp_stack[SZ_4K * NCPU] = {0};

int hyper_init_primary()
{
    int x;
    x = 0x1234;
    return 0;
}

