// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm/csr.h>
#include <asm/encoding.h>
#include <asm/sbi.h>
#include <common.h>
#include <cpu_func.h>
#include <malloc.h>
#include <linux/log2.h>

#define DPMA			(_AC(0x1, UL) << 30)
int pma_set(unsigned long addr, unsigned int size);


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
