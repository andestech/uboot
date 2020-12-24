// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <asm/encoding.h>
#include <malloc.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/cache.h>
#include <dm/uclass-internal.h>
#include <cache.h>
#include <asm/csr.h>
#include <asm/sbi.h>
#include <linux/log2.h>

/* mcache_ctl register*/
#define MCACHE_CTL_IC_EN	(1UL << 0)
#define MCACHE_CTL_DC_EN	(1UL << 1)

//#if CONFIG_IS_ENABLED(RISCV_MMODE)
/* mcctlcommand */
#define CCTL_REG_MCCTLCOMMAND_NUM	0x7cc

/* D-cache operation */
#define CCTL_L1D_WBINVAL_ALL	6
//#endif

#if !CONFIG_IS_ENABLED(RISCV_SMODE)
#define DPMA			(_AC(0x1, UL) << 30)
int pma_set(unsigned long addr, unsigned int size);
#endif

#ifdef CONFIG_V5L2_CACHE
static void _cache_enable(void)
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		cache_enable(dev);
}

static void _cache_disable(void)
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		cache_disable(dev);
}
#endif

void flush_dcache_all(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	if(dcache_status())
		sbi_dcache_wbinval_all();
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */
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
#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	sbi_en_icache();
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */
}

void icache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	asm volatile (
		"fence.i\n\t"
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	sbi_dis_icache();
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */
}

void dcache_enable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	sbi_en_dcache();
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */

#ifdef CONFIG_V5L2_CACHE
	_cache_enable();
#endif
}

void dcache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	sbi_dis_dcache();
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */

#ifdef CONFIG_V5L2_CACHE
	_cache_disable();
#endif
}

int icache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	ret = (sbi_get_L1cache()& MCACHE_CTL_IC_EN);
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */

	return ret;
}

int dcache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE) /* CONFIG_IS_ENABLED(RISCV_MMODE) */
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif
#else /* CONFIG_IS_ENABLED(RISCV_MMODE) */
	ret = (sbi_get_L1cache()& MCACHE_CTL_DC_EN);
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */

	return ret;
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
/*
 * Reserve one MMU section worth of address space below the malloc() area that
 * will be mapped uncached.
 */
static unsigned long noncached_start = 0;
static unsigned long noncached_end = 0;
static unsigned long noncached_next = 0;

int noncached_init(void)
{
	phys_addr_t start, end;
	size_t size;

#if !CONFIG_IS_ENABLED(RISCV_SMODE)
	if (!(csr_read(CSR_MMSCCFG) & DPMA))
#else

	if (!sbi_probe_pma())
#endif
		return -ENXIO;

	end = ALIGN(mem_malloc_start, MMU_SECTION_SIZE) - MMU_SECTION_SIZE;
	size = ALIGN(CONFIG_SYS_NONCACHED_MEMORY, MMU_SECTION_SIZE);
	start = end - size;
	debug("mapping memory %pa-%pa non-cached\n", &start, &end);

	noncached_start = start;
	noncached_end = end;
	noncached_next = start;

	return 0;
}

phys_addr_t noncached_alloc(size_t size, size_t align)
{
	phys_addr_t next = ALIGN(noncached_next, align);
	size = __roundup_pow_of_two(size);

	if (next >= noncached_end || (noncached_end - next) < size)
		return 0;

	debug("allocated %zu bytes of uncached memory @%pa\n", size, &next);
#if !CONFIG_IS_ENABLED(RISCV_SMODE)
	pma_set(next, size);
#else
	sbi_set_pma(next, next, 0x100);
#endif
	noncached_next = next + size;

	return next;
}
#endif /* CONFIG_SYS_NONCACHED_MEMORY */
