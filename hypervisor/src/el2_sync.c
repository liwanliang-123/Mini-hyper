#include "arch.h"
#include "printf.h"
#include "types.h"
#include "hyper_log.h"
#include "vcpu.h"
#include "vpsci.h"
#include "vmmio.h"
#include "gicv3.h"
#include "pl011.h"

void el2_irq_proc(void)
{
    u32 iar, irq;
    gicv3_ops.get_irq(&iar);
    irq = iar & 0x3FF;

    switch(irq) {
        case UART_IRQ_LINE:
            pl011_irq_handler();
            break;
        default:
            LOG_WARN("Unknow IRQ under EL2, irq is %d\n", irq);
            break;
    }

    gicv3_ops.hyp_eoi(irq);
}
