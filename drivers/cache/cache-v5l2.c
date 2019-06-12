// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <asm/v5l2cache.h>

DECLARE_GLOBAL_DATA_PTR;

void v5l2_enable(void)
{
	struct l2cache *regs = gd->arch.v5l2;

	if (regs)
		setbits_le32(&regs->control, L2_ENABLE);
}

void v5l2_disable(void)
{
	volatile struct l2cache *regs = gd->arch.v5l2;
	u8 hart = gd->arch.boot_hart;
	void __iomem *cctlcmd = (void __iomem *)CCTL_CMD_REG(gd->arch.v5l2, hart);

	if ((regs) && (readl(&regs->control) & L2_ENABLE)) {
		writel(L2_WBINVAL_ALL, cctlcmd);

		while ((readl(&regs->cctl_status) & CCTL_STATUS_MSK(hart))) {
			if ((readl(&regs->cctl_status) & CCTL_STATUS_ILLEGAL(hart))) {
				printf("L2 flush illegal! hanging...");
				hang();
			}
		}
		clrbits_le32(&regs->control, L2_ENABLE);
	}
}

static void v5l2_of_parse_and_init(struct udevice *dev)
{
	struct l2cache *regs;
	u32 ctl_val, prefetch;
	u32 tram_ctl[2];
	u32 dram_ctl[2];

	regs = (struct l2cache *)dev_read_addr(dev);

	gd->arch.v5l2 = regs;
	ctl_val = readl(&regs->control);

	if (!(ctl_val & L2_ENABLE))
		ctl_val |= L2_ENABLE;

	/* Instruction and data fetch prefetch depth */
	if (!dev_read_u32(dev, "andes,inst-prefetch", &prefetch)) {
		ctl_val &= ~(IPREPETCH_MSK);
		ctl_val |= (prefetch << IPREPETCH_OFF);
	}

	if (!dev_read_u32(dev, "andes,data-prefetch", &prefetch)) {
		ctl_val &= ~(DPREPETCH_MSK);
		ctl_val |= (prefetch << DPREPETCH_OFF);
	}

	/* Set tag RAM and data RAM setup and output cycle */
	if (!dev_read_u32_array(dev, "andes,tag-ram-ctl", tram_ctl, 2)) {
		ctl_val &= ~(TRAMOCTL_MSK | TRAMICTL_MSK);
		ctl_val |= tram_ctl[0] << TRAMOCTL_OFF;
		ctl_val |= tram_ctl[1] << TRAMICTL_OFF;
	}

	if (!dev_read_u32_array(dev, "andes,data-ram-ctl", dram_ctl, 2)) {
		ctl_val &= ~(DRAMOCTL_MSK | DRAMICTL_MSK);
		ctl_val |= dram_ctl[0] << DRAMOCTL_OFF;
		ctl_val |= dram_ctl[1] << DRAMICTL_OFF;
	}

	writel(ctl_val, &regs->control);
}

static int v5l2_probe(struct udevice *dev)
{
	v5l2_of_parse_and_init(dev);

	return 0;
}

static const struct udevice_id v5l2_cache_ids[] = {
	{ .compatible = "cache" },
	{}
};

U_BOOT_DRIVER(v5l2_cache) = {
	.name   = "v5l2_cache",
	.id     = UCLASS_CACHE,
	.of_match = v5l2_cache_ids,
	.probe	= v5l2_probe,
	.flags  = DM_FLAG_PRE_RELOC,
};
