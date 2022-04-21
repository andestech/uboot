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

/* MMISC control register */
#define V5_MMISC_CTL_NON_BLOCKING_OFFSET		8
#define V5_MMISC_CTL_NON_BLOCKING_EN			BIT(V5_MMISC_CTL_NON_BLOCKING_OFFSET)

/* MCACHE control register */
#define V5_MCACHE_CTL_IC_EN_OFFSET				0
#define V5_MCACHE_CTL_DC_EN_OFFSET				1
#define V5_MCACHE_CTL_CCTL_SUEN_OFFSET			8
#define V5_MCACHE_CTL_L1I_PREFETCH_OFFSET		9
#define V5_MCACHE_CTL_L1D_PREFETCH_OFFSET		10
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_1		13
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_2		14
#define V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1		15
#define V5_MCACHE_CTL_L2C_WAROUND_OFFSET_2		16
#define V5_MCACHE_CTL_DC_COHEN_OFFSET			19
#define V5_MCACHE_CTL_DC_COHSTA_OFFSET			20

#define V5_MCACHE_CTL_IC_EN						BIT(V5_MCACHE_CTL_IC_EN_OFFSET)
#define V5_MCACHE_CTL_DC_EN						BIT(V5_MCACHE_CTL_DC_EN_OFFSET)
#define V5_MCACHE_CTL_CCTL_SUEN					BIT(V5_MCACHE_CTL_CCTL_SUEN_OFFSET)
#define V5_MCACHE_CTL_L1I_PREFETCH_EN			BIT(V5_MCACHE_CTL_L1I_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_L1D_PREFETCH_EN			BIT(V5_MCACHE_CTL_L1D_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_DC_WAROUND_1_EN			BIT(V5_MCACHE_CTL_DC_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_DC_WAROUND_2_EN			BIT(V5_MCACHE_CTL_DC_WAROUND_OFFSET_2)
#define V5_MCACHE_CTL_L2C_WAROUND_1_EN			BIT(V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_L2C_WAROUND_2_EN			BIT(V5_MCACHE_CTL_L2C_WAROUND_OFFSET_2)
#define V5_MCACHE_CTL_DC_COHEN_EN				BIT(V5_MCACHE_CTL_DC_COHEN_OFFSET)
#define V5_MCACHE_CTL_DC_COHSTA_EN				BIT(V5_MCACHE_CTL_DC_COHSTA_OFFSET)

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

	return 0;
}

void harts_early_init(void)
{
	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
        unsigned long long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);
        unsigned long long mmisc_ctl_val = csr_read(CSR_MMISC_CTL);

		mcache_ctl_val |= (V5_MCACHE_CTL_DC_COHEN_EN | V5_MCACHE_CTL_IC_EN | \
							V5_MCACHE_CTL_DC_EN | V5_MCACHE_CTL_CCTL_SUEN | \
							V5_MCACHE_CTL_L1I_PREFETCH_EN | V5_MCACHE_CTL_L1D_PREFETCH_EN | \
							V5_MCACHE_CTL_DC_WAROUND_1_EN | V5_MCACHE_CTL_L2C_WAROUND_1_EN);

		csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

		/*
         * Check DC_COHEN_EN, if cannot write to mcache_ctl,
         * we assume this bitmap not support L2 CM
         */
        mcache_ctl_val = csr_read(CSR_MCACHE_CTL);
        if ((mcache_ctl_val & V5_MCACHE_CTL_DC_COHEN_EN)) {

			/* Wait for DC_COHSTA bit be set */
			while (!(mcache_ctl_val & V5_MCACHE_CTL_DC_COHSTA_EN))
				mcache_ctl_val = csr_read(CSR_MCACHE_CTL);
		}

		if (!(mmisc_ctl_val & V5_MMISC_CTL_NON_BLOCKING_EN))
			mmisc_ctl_val |= V5_MMISC_CTL_NON_BLOCKING_EN;

		csr_write(CSR_MMISC_CTL, mmisc_ctl_val);
	}
}
