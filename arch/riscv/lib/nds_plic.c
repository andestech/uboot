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
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
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

#define ENABLE_HART_IPI         (0x80808080)

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
static int enable_ipi(int hart)
{
	int en;
	void *addr;
	gd_t *pgd;

	en = ENABLE_HART_IPI >> hart;
	addr = (void *)gd->old_gd + ((1<<RISCV_PGSHIFT)*hart);
	pgd = (gd_t *)addr;
	writel(en, (void __iomem *)ENABLE_REG(gd->arch.plic, hart));
	pgd->arch.plic = gd->arch.plic;
	pgd->plic_sw.claim[hart] = (void __iomem *)CLAIM_REG(gd->arch.plic, hart);

	return 0;
}

int init_plic(void)
{
		struct udevice *dev;
		ofnode node;
		int ret;
		int reg;
		int num = 0;

		ret = uclass_find_first_device(UCLASS_CPU, &dev);

		if (ret)
			return ret;

		if (ret == 0 && dev) {
			ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
				const char *device_type;

				device_type = ofnode_read_string(node, "device_type");

				if (!device_type)
					continue;

				if (strcmp(device_type, "cpu"))
					continue;

				/* skip if hart is marked as not available in the device tree */
				if (!ofnode_is_available(node))
					continue;

				/* read hart ID of CPU */
				ret = ofnode_read_u32(node, "reg", &reg);

				if (ret == 0) {
					num++;
					enable_ipi(reg);
				}
			}
			gd->arch.hart_num = num;

			return 0;
		}

		return -ENODEV;
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
