#include "layout.h"
#include "printf.h"
#include "hyper_log.h"
#include "spinlock.h"
#include "malloc.h"
#include "kalloc.h"
#include "gicv3.h"
#include "vmm.h"
#include "vm.h"

__attribute__((aligned(SZ_4K))) char sp_stack[SZ_4K * NCPU] = {0};

extern void test_create_vm_mapping(void);
extern guest_t guest_vm_image;
extern guest_t guest_virt_dtb;
extern guest_t guest_rootfs;

static void print_logo(void)
{
    printf("Welcome to Mini-Hyper OS!\n");
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

    stage2_mmu_init();
    hyper_setup();

    pcpu_init();
    vcpu_init();
    LOG_INFO("Pcpu/vcpu arrays have been initialized\n");

    vm_config_t guest_vm_cfg = {
        .guest_image  = &guest_vm_image,
        .guest_dtb    = &guest_virt_dtb,
        .guest_initrd = &guest_rootfs,
        .entry_addr   = 0x80600000,
        .dtb_addr     = 0x80000000,  /* virt dtb ipa */
        .rootfs_addr  = 0x84000000,  /* rootfs ipa */
        .ram_size     = 0x8000000,   /* 128M */
        .ncpu         = 2,
    };

    create_guest_vm(&guest_vm_cfg);
    start_vcpu();

    while(1) {}

    return 0;
}
