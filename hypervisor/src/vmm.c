#include "printf.h"
#include "types.h"
#include "layout.h"
#include "arch.h"
#include "kalloc.h"
#include "vmm.h"
#include "hyper_log.h"
#include "utils.h"

u64 *page_walk(u64 *pgt, u64 va, bool alloc)
{
    /* page walk from L0 ~ L2 */
    for(int table_level = 0; table_level < 3; table_level ++) {
        u64 *pte = &pgt[PINDEX(table_level, va)];

        /* Page table entry has been mapped to next-level Page table */
        if((*pte & PTE_VALID) && (*pte & PTE_TABLE)) {
            /* Next-Table address has been alloced
             * 63        47                       12          2   1   0
             * +--------+--------------------------+----------+---+---+
             * |        | Next-level table addr    | IGNORED  | 1 | 1 |
             * +--------+--------------------------+----------+---+---+
             */
            /* Get next-level page table address */
            pgt = (u64 *)PTE_PA(*pte);
        } else if(alloc == true) {
            /* Page table entry is not mapped, alloc one page from next-level page table */
            pgt = alloc_one_page();
            if(pgt == NULL) {
                abort("Unable to alloc one page for page_walk");
            }
            *pte = PTE_PA(pgt) | PTE_TABLE | PTE_VALID;
        } else {
            return NULL;
        }
    }

    /* L3 PTE */
    return &pgt[PINDEX(3, va)];
}

void create_guest_mapping(u64 *pgt, u64 va, u64 pa, u64 size, u64 mattr)
{
    /* va, pa, size shall be page alignment */
    if(va % PAGESIZE != 0 ||
       pa % PAGESIZE != 0 ||
       size % PAGESIZE != 0) {
        abort("Create_guest_mapping with invalid param");
    }

    for(u64 p = 0; p < size; p += PAGESIZE, va += PAGESIZE, pa += PAGESIZE) {
        u64 *pte = page_walk(pgt, va, true);
        if(*pte & PTE_AF) {
            abort("Page table entry has been used");
        }
        *pte = PTE_PA(pa) | PTE_AF | mattr | PTE_V;
    }
}

void page_unmap(u64 *pgt, u64 va, u64 size)
{
    if(va % PAGESIZE != 0 || size % PAGESIZE != 0) {
        abort("Page_unmap with invalid param");
    }

    for(u64 p = 0; p < size; p += PAGESIZE, va += PAGESIZE) {
        u64 *pte = page_walk(pgt, va, false);
        if(*pte == 0) {
            abort("Page already unmapped");
        }
        u64 pa = PTE_PA(*pte);
        free_one_page((void *)pa);
        *pte = 0;
    }
}

u64 ipa_to_pa(u64 *pgt, u64 ipa)
{
    u64 *pte = page_walk(pgt, ipa, 0);
    if(pte == NULL) {
        return 0;
    }

    u32 offset = ipa & (PAGESIZE - 1);
    return PTE_PA(*pte) + offset;
}

void copy_to_ipa(u64 *pgt, u64 to_ipa, char *from, u64 len)
{
    while(len > 0) {
        u64 pa = ipa_to_pa(pgt, to_ipa);
        if(pa == 0) {
            abort("Copy to ipa failed to find pte");
        }
        u64 poff = to_ipa & (PAGESIZE - 1);
        u64 n = PAGESIZE - poff;
        if(n > len) {
            n = len;
        }

        memcpy((char *)pa, from, n);
        from += n;
        to_ipa += n;
        len -= n;
    }
}

void stage2_mmu_init(void)
{
    LOG_INFO("Stage2 Translation MMU initialization ...\n");

    /* Physical Address range supported */
    u64 feature;
    read_sysreg(feature, id_aa64mmfr0_el1);
    LOG_INFO("PARange bits is %d\n", PA_RANGE(feature));

    /*
     * T0SZ = 64 - 20 = 44 : (IPA range is 2^(64-20) = 2^44)
     * SL0  = 2 : starting level is leve-0
     * TG0  = 0 : 4K Granule size
     * PS   = 1 : Physical address Size is 36 bits
     */
    u64 vtcr = VTCR_T0SZ(20) | VTCR_SL0(2) |
               VTCR_SH0(0) | VTCR_TG0(0) | VTCR_NSW |
               VTCR_NSA | VTCR_PS(4);

    LOG_INFO("Setting vtcr_el2 to 0x%x\n", vtcr);
    write_sysreg(vtcr_el2, vtcr);

    u64 mair = (DEVICE_nGnRnE << (8 * DEVICE_nGnRnE_INDEX)) | (NORMAL_NC << (8 * NORMAL_NC_INDEX));
    LOG_INFO("Setting mair_el2 to 0x%x\n", mair);
    write_sysreg(mair_el2, mair);

    isb();

    return;
}

/* Provides configuration controls for virtualization */
extern void hyper_vector();
void hyper_setup()
{
    /* HCR_TSC: Traps EL1 execution of SMC instructions to EL2
     * HCR_RW:  The Execution state for EL1 is AArch64
     * HCR_VM:  EL1&0 stage 2 address translation enabled
     */
    u64 hcr = HCR_TSC | HCR_RW | HCR_FMO | HCR_IMO | HCR_VM;
    LOG_INFO("Setting hcr_el2 to 0x%x (TSC|RW|FMO|IMO|VM)\n", hcr);
    write_sysreg(hcr_el2, hcr);

    LOG_INFO("Setting Vector Base Address Register for EL2\n");
    write_sysreg(vbar_el2, (u64)hyper_vector);

    isb();
}
