#ifndef __VM_H__
#define __VM_H__

#include "types.h"
#include "guest.h"
#include "layout.h"
#include "spinlock.h"

struct vmmio_access;

typedef struct vm_config {
    guest_t *guest_image;
    guest_t *guest_dtb;
    guest_t *guest_initrd;
    u64      entry_addr;
    u64      ram_size;
    int      ncpu;
    u64      dtb_addr;
    u64      rootfs_addr;
} vm_config_t;

typedef struct vm {
    char       name[32];
    int        nvcpu;
    u64       *vttbr;
    spinlock_t vm_lock;
    struct vcpu *vcpus[NCPU];
    struct vgicv3_dist *vgic_dist;
    struct vmmio_info *vmmios;
    u64    dtb;
} vm_t;


#endif
