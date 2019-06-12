// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

/* CPU specific code */
#include <common.h>
#include <asm/cache.h>
#include <asm/v5l2cache.h>
#include <dm.h>
#include <dm/util.h>
#include <dm/lists.h>

extern long *init_plic(int c);

struct ae350_soc_simple {
	void __iomem *regs;
};

struct ae350_plic {
	void __iomem *regs;
};

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	/* turn off I/D-cache */
	cache_flush();
	icache_disable();
	dcache_disable();
#ifdef CONFIG_RISCV_NDS_CACHE
	v5l2_disable();
#endif

	return 0;
}

static int ae350_soc_init(void __iomem *regs)
{
	return 0;
}

static int ae350_soc_probe(struct udevice *dev)
{
	struct ae350_soc_simple *simple = dev_get_platdata(dev);
	fdt_addr_t base;
	int (*init)(void __iomem *regs);
	int ret;

	base = devfdt_get_addr(dev);
	simple->regs = (void __iomem *)base;
	init = (typeof(init))dev_get_driver_data(dev);	
	ret = init(simple->regs);

	return ret;
}

static int plic_probe(struct udevice *dev)
{
	return 0;
}

int plic_bind(struct udevice *dev, const char *drv_name)
{
#ifdef CONFIG_SMP
	const void *fdt = gd->fdt_blob;
	int offset = dev_of_offset(dev);
	bool pre_reloc_only = !(gd->flags & GD_FLG_RELOC);
	const char *name;
	int ret;
	struct ae350_plic *plat = dev_get_platdata(dev);
	struct udevice *child = 0;
	ofnode	node;
	int ndev;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		if (pre_reloc_only &&
		    !dm_fdt_pre_reloc(fdt, offset))
			continue;
		/*
		 * If this node has "compatible" property, this is not
		 * a clock sub-node, but a normal device. skip.
		 */
		fdt_get_property(fdt, offset, "compatible", &ret);

		if (!fdt_node_check_compatible(fdt, offset, "riscv,plic1")) {
			node = offset_to_ofnode(offset);
			/* IMPORTANT: after lists_bind_fdt, dev will have child */
			lists_bind_fdt(dev, node, &child, false);
			plat->regs = (void __iomem *)devfdt_get_addr(child);
			gd->arch.plic = plat->regs;
			dev_read_u32(child, "riscv,ndev", (u32 *)&ndev);

			init_plic(ndev);
			gd->arch.hart_num = ndev;
		}
		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;

		ret = device_bind_driver_to_node(dev, drv_name, name,
					offset_to_ofnode(offset), NULL);
		if (ret)
			return ret;
	}
#endif

	return 0;
}

static int ae350_soc_bind(struct udevice *dev)
{
	return plic_bind(dev, "plic1");
}


/* To enumerate devices on the /soc/ node, create a "simple-bus" driver */
static const struct udevice_id riscv_ae350_soc_ids[] = {
	{
		.compatible = "andestech,riscv-ae350-soc",
		.data = (ulong)ae350_soc_init,
	},
	{ }
};

U_BOOT_DRIVER(riscv_ae350_soc_ids) = {
	.name = "andestech,riscv-ae360-soc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = riscv_ae350_soc_ids,
	.probe = ae350_soc_probe,
	.bind = ae350_soc_bind,
	.platdata_auto_alloc_size = sizeof(struct ae350_soc_simple),
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(plic) = {
	.name = "plic1",
	.id = UCLASS_CLK,
	.probe = plic_probe,
	.platdata_auto_alloc_size = sizeof(struct ae350_plic),
	.flags = DM_FLAG_PRE_RELOC,
};
