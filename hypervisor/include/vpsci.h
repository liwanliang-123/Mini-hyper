#ifndef __VPSCI_H__
#define __VPSCI_H__

#include "types.h"
#include "vcpu.h"

/* https://developer.aliyun.com/article/1205031 */
#define PSCI_VERSION            0x84000000
#define PSCI_MIGRATE_INFO_TYPE  0x84000006
#define PSCI_SYSTEM_OFF         0x84000008
#define PSCI_SYSTEM_RESET       0x84000009
#define PSCI_SYSTEM_CPUON       0xc4000003
#define PSCI_FEATURE		    0x8400000a
#define PSCI_CPU_OFF            0x84000002
#define PSCI_AFFINITY_INFO      0xc4000004

#define PSCI_0_2_AFFINITY_LEVEL_ON	    	0
#define PSCI_0_2_AFFINITY_LEVEL_OFF	    	1
#define PSCI_0_2_AFFINITY_LEVEL_ON_PENDING	2

u64 vpsci_trap_smc(vcpu_t *vcpu, u64 funid, u64 target_cpu, u64 entry_addr);
u64 smc_call(u64 funid, u64 target_cpu, u64 entry_addr);

#endif

