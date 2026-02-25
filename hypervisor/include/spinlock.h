#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include "types.h"
#include "arch.h"

typedef struct spinlock {
    int   coreid;
    u8    lock;
    char  *name;
} spinlock_t;

static inline int spin_check(spinlock_t *spinlock)
{
    if(spinlock->lock && spinlock->coreid == coreid()) {
        /* spinlock is currently held by the calling CPU */
        return 1;
    } else {
        /* spinlock is unlocked or
         * spinlock is not currently held by the calling CPU
         */
        return 0;
    }
}

static inline void __arch_spinlock_init(spinlock_t *spinlock, char *name)
{
    spinlock->coreid = -1;
    spinlock->lock   = 0;
    spinlock->name   = name;
}

#define arch_spinlock_init(lock) __arch_spinlock_init(lock, #lock)

void arch_spin_lock(spinlock_t *spinlock);
void arch_spin_unlock(spinlock_t *spinlock);

#endif
