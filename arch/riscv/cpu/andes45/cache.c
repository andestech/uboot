// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm/encoding.h>
#include <asm/csr.h>
#include <asm/sbi.h>
#include <cache.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <linux/log2.h>

#define MCACHE_CTL	0x7ca
#define MCACHE_CTL_IC_EN	(1UL << 0)
#define MCACHE_CTL_DC_EN	(1UL << 1)

#define CCTL_REG_MCCTLCOMMAND_NUM	0x7cc

#define CCTL_L1D_WBINVAL_ALL	6

void enable_caches(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CACHE,
					  DM_DRIVER_GET(v5l2_cache),
					  &dev);
	if (ret) {
		log_debug("Cannot enable v5l2 cache");
	} else {
		ret = cache_enable(dev);
		if (ret)
			log_debug("v5l2 cache enable failed");
	}
}

static void cache_ops(int (*ops)(struct udevice *dev))
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		ops(dev);
}


void flush_dcache_all(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
#else
	if(dcache_status())
		sbi_dcache_wbinval_all();
#endif

	cache_ops(cache_wbinval);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void icache_enable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, 0x7ca\n\t"
		"ori t0, t1, 0x1\n\t"
		"csrw 0x7ca, t0\n\t"
	);
#else
	sbi_en_icache();
#endif
}

void icache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"fence.i\n\t"
		"csrr t1, 0x7ca\n\t"
		"andi t0, t1, ~0x1\n\t"
		"csrw 0x7ca, t0\n\t"
	);
#else
	sbi_dis_icache();
#endif
}

void dcache_enable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, 0x7ca\n\t"
		"ori t0, t1, 0x2\n\t"
		"csrw 0x7ca, t0\n\t"
	);
#else
	if (!(sbi_get_L1cache()& MCACHE_CTL_DC_EN))
		sbi_en_dcache();
#endif
	cache_ops(cache_enable);
}

void dcache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
	asm volatile (
		"csrr t1, 0x7ca\n\t"
		"andi t0, t1, ~0x2\n\t"
		"csrw 0x7ca, t0\n\t"
	);
#else
	sbi_dis_dcache();
#endif
	cache_ops(cache_disable);
}

int icache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, 0x7ca\n\t"
		"andi	%0, t1, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#else
	ret = (sbi_get_L1cache()& MCACHE_CTL_IC_EN);
#endif

	return ret;
}

int dcache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, 0x7ca\n\t"
		"andi	%0, t1, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#else
	ret = (sbi_get_L1cache()& MCACHE_CTL_DC_EN);
#endif

	return ret;
}
