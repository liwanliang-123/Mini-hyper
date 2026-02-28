#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer_init(void);
uint64_t timer_get_us(void);

#endif
