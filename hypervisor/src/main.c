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

static inline int get_current_el(void)
{
    int el;
    __asm__ volatile("mrs %0, CurrentEL" : "=r" (el));
    return (el >> 2) & 0x3;
}

static void print_logo(void)
{
    // printf("\033[1;32m🐧\033[0m Welcome to Mini-Hyper OS, running on EL%d\n", get_current_el());
    printf("\033[1;32m🐧\033[0m Welcome to Mini-Hyper — a Type-1 Hypervisor OS.\n");
    printf("\n");
    printf("   .--.\n");
    printf("  |o_o |\n");
    printf("  |:_/ |\n");
    printf(" //   \\ \\\n");
    printf("(|     | )\n");
    printf("/'\\_   _/`\\\n");
    printf("\\___)=(___/\n");
    printf("\n");
    printf("\033[1;32m✓\033[0m Starting initializing system ...");
    printf("\n");
}

int hyper_init_secondary()
{
    LOG_INFO("pcpu %d is activated\n", coreid());

    gic_percpu_init();
    irq_enable;
    stage2_mmu_init();
    hyper_setup();

    start_vcpu();
    return 0;
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
    // LOG_INFO("Pcpu/vcpu arrays have been initialized\n");

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
