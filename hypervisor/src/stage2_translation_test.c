#include "hyper_log.h"
#include "vmm.h"
#include "kalloc.h"
#include "types.h"
#include "printf.h"
#include "layout.h"
#include "guest.h"
#include "utils.h"
#include "vm.h"
#include "arch.h"

extern guest_t guest_vm_image;

vm_config_t guest_vm_cfg = {
    .guest_image  = &guest_vm_image,
    .guest_dtb    = NULL,
    .guest_initrd = NULL,
    .entry_addr   = 0x80200000,
    .ram_size     = 0x80000,
};

void test_create_vm_mapping(void)
{
    LOG_INFO("(Testing) create stage2 memory mapping\n");

    LOG_INFO("(Testing) guest image name  is %s\n", guest_vm_cfg.guest_image->guest_name);
    LOG_INFO("(Testing) guest image start is %x\n", guest_vm_cfg.guest_image->start_addr);
    LOG_INFO("(Testing) guest image end   is %x\n", guest_vm_cfg.guest_image->end_addr);
    LOG_INFO("(Testing) guest image size  is %x\n", guest_vm_cfg.guest_image->image_size);
    LOG_INFO("(Testing) guest vm entry    is %x\n", guest_vm_cfg.entry_addr);

    /* alloc a page for vttbr */
    LOG_INFO("(Testing) Alloc a page for vttbr\n");
    u64 *vttbr = alloc_one_page();
    if(vttbr == NULL) {
        abort("Unable to alloc a page");
    }

    /* create normal range mapping for guest */
    u64   p;
    char *page;

    /* create guest image mapping */
    LOG_INFO("(Testing) Create guest image mapping\n");
    u64 copy_size;
    for(p = 0; p < guest_vm_cfg.guest_image->image_size; p += PAGESIZE) {
        char *page = alloc_one_page();
        if(page == NULL) {
            abort("Unable to alloc a page");
        }

        if(guest_vm_cfg.guest_image->image_size - p > PAGESIZE) {
            copy_size = PAGESIZE;
        } else {
            copy_size = guest_vm_cfg.guest_image->image_size - p;
        }
        /* copy the guest image content from Mini-Hyper image to pages */
        memcpy(page, (char *)guest_vm_cfg.guest_image->start_addr + p, copy_size);
        create_guest_mapping(vttbr, guest_vm_cfg.entry_addr + p, (u64)page, PAGESIZE, S2PTE_NORMAL | S2PTE_RW);
    }

    LOG_INFO("(Testing) Create normal range mapping for guest\n");
    for( ; p < guest_vm_cfg.ram_size; p += PAGESIZE) {
        /* Alloc a page size physical memory */
        page = alloc_one_page();
        if(page == NULL) {
            abort("Unable to alloc a page");
        }
        create_guest_mapping(vttbr, guest_vm_cfg.entry_addr + p, (u64)page, PAGESIZE, S2PTE_NORMAL | S2PTE_RW);
    }

    /* Uart device passthrough */
    create_guest_mapping(vttbr, PL011BASE, PL011BASE, PAGESIZE, S2PTE_DEVICE | S2PTE_RW);

    write_sysreg(vttbr_el2, vttbr);
    flush_tlb();

    /* eret to EL1 and use SP_EL1 and enable DAIF */
    write_sysreg(spsr_el2, 0x3c5);
    /* eret address on EL1 IPA address */
    write_sysreg(elr_el2,  guest_vm_cfg.entry_addr);
    isb();

    asm volatile("eret");

    return;
}
