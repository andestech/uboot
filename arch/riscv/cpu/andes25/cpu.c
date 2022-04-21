// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

/* CPU specific code */
#include <common.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/cache.h>
#include <asm/csr.h>

#define CSR_MCACHE_CTL							0x7ca
#define CSR_MMISC_CTL							0x7d0
#define V5_MCACHE_CTL_IC_EN_OFFSET		0
#define V5_MCACHE_CTL_DC_EN_OFFSET		1
#define V5_MCACHE_CTL_CCTL_SUEN_OFFSET  8

#define V5_MCACHE_CTL_IC_EN			BIT(V5_MCACHE_CTL_IC_EN_OFFSET)
#define V5_MCACHE_CTL_DC_EN			BIT(V5_MCACHE_CTL_DC_EN_OFFSET)
#define V5_MCACHE_CTL_CCTL_SUEN	BIT(V5_MCACHE_CTL_CCTL_SUEN_OFFSET)

/* MMISC control register */
#define V5_MMISC_CTL_NON_BLOCKING_OFFSET		8
#define V5_MMISC_CTL_NON_BLOCKING_EN			BIT(V5_MMISC_CTL_NON_BLOCKING_OFFSET)

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	invalidate_icache_all();
	if(!icache_status())
		icache_enable();

	if(!dcache_status())
		dcache_enable();

	enable_caches();

	return 0;
}

void harts_early_init(void)
{
	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
        unsigned long long mmisc_ctl_val = csr_read(CSR_MMISC_CTL);
		unsigned long long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);

		if (!(mcache_ctl_val & V5_MCACHE_CTL_CCTL_SUEN))
			mcache_ctl_val |= V5_MCACHE_CTL_CCTL_SUEN;
		if (!(mcache_ctl_val & V5_MCACHE_CTL_IC_EN))
			mcache_ctl_val |= V5_MCACHE_CTL_IC_EN;

		if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF)) {
			if (!(mcache_ctl_val & V5_MCACHE_CTL_DC_EN))
				mcache_ctl_val |= V5_MCACHE_CTL_DC_EN;
		}

		csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

		if (!(mmisc_ctl_val & V5_MMISC_CTL_NON_BLOCKING_EN))
			mmisc_ctl_val |= V5_MMISC_CTL_NON_BLOCKING_EN;

		csr_write(CSR_MMISC_CTL, mmisc_ctl_val);
	}
}
