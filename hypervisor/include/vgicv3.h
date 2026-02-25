#ifndef __VGICV3_H__
#define __VGICV3_H__

#include "types.h"
#include "gicv3.h"
#include "spinlock.h"
#include "vcpu.h"
#include "vm.h"

struct vm;
struct vcpu;

/* One irq configuration */
struct vgicv3_irq_config {
    u8 priority;
    u8 affinity;
    u8 enabled;
    u8 group;
};

/* virtual gic distributor */
struct vgicv3_dist {
    u32 nspis;
    u8  enabled;
    struct vgicv3_irq_config *spis;
    spinlock_t lock;
};

/* virtual gic cpu interface */
struct vgicv3_cpu {
    u16 used_lr;
    struct vgicv3_irq_config sgis[GIC_NSGI];
    struct vgicv3_irq_config ppis[GIC_NPPI];
};

struct vgicv3_cpu *create_vgic_cpu(int vcpuid);
struct vgicv3_dist *create_vgic_dist(struct vm *vm);
void virq_enter(struct vcpu *vcpu);
int  virq_inject(struct vcpu *vcpu, u32 pirq, u32 virq);
int  vgicv3_generate_sgi(struct vcpu *vcpu, int rt, int wr);

#endif
