// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Rick Chen <rick@andestech.com>
 *
 * U-Boot syscon driver for Andes's Platform Level Interrupt Controller (PLIC).
 * The PLIC block holds memory-mapped control and pending registers
 * associated with software interrupts.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <asm/encoding.h>

/* pending registers */
#define PENDING_REG(base, hart)	((ulong)(base) + 0x1000 + (hart) * 8)
/* enable register */
#define ENABLE_REG(base, hart)	((ulong)(base) + 0x2000 + (hart) * 0x80)
/* claim register */
#define CLAIM_REG(base, hart)	((ulong)(base) + 0x200004 + (hart) * 0x1000)

DECLARE_GLOBAL_DATA_PTR;

#define PLIC_BASE_GET(void)						\
	do {								\
		long *ret;						\
									\
		if (!gd->arch.plic) {					\
			ret = syscon_get_first_range(RISCV_SYSCON_PLIC); \
			if (IS_ERR(ret))				\
				return PTR_ERR(ret);			\
			gd->arch.plic = ret;				\
		}							\
	} while (0)

#if CONFIG_SMP
int init_plic(int c)
{
	int i;
	int en = 0x80808080;
	void *addr;
	gd_t *pgd;

	PLIC_BASE_GET();
	for(i=0;i<c;i++)
	{
		en = en >> i;
		addr = (void *)gd->old_gd + ((1<<RISCV_PGSHIFT)*i);
		pgd = (gd_t *)addr;
		writel(en, (void __iomem *)ENABLE_REG(gd->arch.plic, i));
		pgd->arch.plic = gd->arch.plic;
		pgd->plic_sw.claim[i] = (void __iomem *)CLAIM_REG(gd->arch.plic, i);
	}

	return 0;
}

void send_ipi(int c)
{
	int i;

	for(i=1;i<c;i++)
		writel(1<<(7-i), (void __iomem *)PENDING_REG(gd->arch.plic, 0));
}

void hart_claim(ulong i)
{
	gd->plic_sw.source_id = readl((void __iomem *)CLAIM_REG(gd->arch.plic, i));
}

void hart_complete(ulong i)
{
	writel(gd->plic_sw.source_id, (void __iomem *)CLAIM_REG(gd->arch.plic, i));	
}

int riscv_get_plic_base(void)
{
	PLIC_BASE_GET();

	return 0;
}

static const struct udevice_id nds_plic_ids[] = {
	{ .compatible = "riscv,plic1", .data = RISCV_SYSCON_PLIC },
	{ }
};

U_BOOT_DRIVER(nds_plic) = {
	.name		= "nds_plic",
	.id		= UCLASS_SYSCON,
	.of_match	= nds_plic_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};
#endif
