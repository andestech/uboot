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
#include <asm/arch-andes/csr.h>

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
        unsigned long long mmsc_cfg_val = csr_read(CSR_MMSC_CFG);
        unsigned long long mdcm_cfg_val = csr_read(CSR_MICM_CFG);
        unsigned long long micm_cfg_val = csr_read(CSR_MDCM_CFG);

		mcache_ctl_val |= (V5_MCACHE_CTL_DC_COHEN_EN | V5_MCACHE_CTL_IC_EN | \
							V5_MCACHE_CTL_DC_EN | V5_MCACHE_CTL_CCTL_SUEN | \
							V5_MCACHE_CTL_L1I_PREFETCH_EN | V5_MCACHE_CTL_L1D_PREFETCH_EN | \
							V5_MCACHE_CTL_DC_WAROUND_1_EN | V5_MCACHE_CTL_L2C_WAROUND_1_EN);

		if ((mmsc_cfg_val & V5_MMSC_CFG_TLB_ECC_1) || (mmsc_cfg_val & V5_MMSC_CFG_TLB_ECC_2))
			mcache_ctl_val |= V5_MCACHE_CTL_TLB_ECCEN_2;
		if ((micm_cfg_val & V5_MICM_CFG_IC_ECC_1) || (micm_cfg_val & V5_MICM_CFG_IC_ECC_2))
			mcache_ctl_val |= V5_MCACHE_CTL_IC_ECCEN_2;
		if ((mdcm_cfg_val & V5_MDCM_CFG_DC_ECC_1) || (mdcm_cfg_val & V5_MDCM_CFG_DC_ECC_1))
			mcache_ctl_val |= V5_MCACHE_CTL_DC_ECCEN_2;

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
