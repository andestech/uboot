// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <asm/csr.h>

#ifdef CONFIG_RISCV_NDS_CACHE
/* mcctlcommand */
#define CCTL_REG_MCCTLCOMMAND_NUM      0x7cc

/* D-cache operation */
#define CCTL_L1D_WBINVAL_ALL	6
#endif

void flush_dcache_all(void)
{
#ifdef CONFIG_RISCV_NDS_CACHE
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
#endif
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
#ifndef CONFIG_SYS_ICACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void icache_disable(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"fence.i\n\t"
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void dcache_enable(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void dcache_disable(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

int icache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif

	return ret;
}

int dcache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif

	return ret;
}
