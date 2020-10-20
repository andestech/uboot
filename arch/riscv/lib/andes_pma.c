/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <asm/encoding.h>
#include <common.h>
#include <linux/log2.h>

char tor_index = 8;
char align_index = 0;

static unsigned long csr_read_num(int csr_num)
{
	unsigned long ret = 0;

	switch (csr_num) {
	case CSR_PMACFG0:
		ret = csr_read(CSR_PMACFG0);
		break;
	case CSR_PMACFG1:
		ret = csr_read(CSR_PMACFG1);
		break;
	case CSR_PMACFG2:
		ret = csr_read(CSR_PMACFG2);
		break;
	case CSR_PMACFG3:
		ret = csr_read(CSR_PMACFG3);
		break;
	case CSR_PMAADDR0:
		ret = csr_read(CSR_PMAADDR0);
		break;
	case CSR_PMAADDR1:
		ret = csr_read(CSR_PMAADDR1);
		break;
	case CSR_PMAADDR2:
		ret = csr_read(CSR_PMAADDR2);
		break;
	case CSR_PMAADDR3:
		ret = csr_read(CSR_PMAADDR3);
		break;
	case CSR_PMAADDR4:
		ret = csr_read(CSR_PMAADDR4);
		break;
	case CSR_PMAADDR5:
		ret = csr_read(CSR_PMAADDR5);
		break;
	case CSR_PMAADDR6:
		ret = csr_read(CSR_PMAADDR6);
		break;
	case CSR_PMAADDR7:
		ret = csr_read(CSR_PMAADDR7);
		break;
	case CSR_PMAADDR8:
		ret = csr_read(CSR_PMAADDR8);
		break;
	case CSR_PMAADDR9:
		ret = csr_read(CSR_PMAADDR9);
		break;
	case CSR_PMAADDR10:
		ret = csr_read(CSR_PMAADDR10);
		break;
	case CSR_PMAADDR11:
		ret = csr_read(CSR_PMAADDR11);
		break;
	case CSR_PMAADDR12:
		ret = csr_read(CSR_PMAADDR12);
		break;
	case CSR_PMAADDR13:
		ret = csr_read(CSR_PMAADDR13);
		break;
	case CSR_PMAADDR14:
		ret = csr_read(CSR_PMAADDR14);
		break;
	case CSR_PMAADDR15:
		ret = csr_read(CSR_PMAADDR15);
		break;
	default:
		break;
	};

	return ret;
}

static void csr_write_num(int csr_num, unsigned long val)
{
	switch (csr_num) {
	case CSR_PMACFG0:
		csr_write(CSR_PMACFG0, val);
		break;
	case CSR_PMACFG1:
		csr_write(CSR_PMACFG1, val);
		break;
	case CSR_PMACFG2:
		csr_write(CSR_PMACFG2, val);
		break;
	case CSR_PMACFG3:
		csr_write(CSR_PMACFG3, val);
		break;
	case CSR_PMAADDR0:
		csr_write(CSR_PMAADDR0, val);
		break;
	case CSR_PMAADDR1:
		csr_write(CSR_PMAADDR1, val);
		break;
	case CSR_PMAADDR2:
		csr_write(CSR_PMAADDR2, val);
		break;
	case CSR_PMAADDR3:
		csr_write(CSR_PMAADDR3, val);
		break;
	case CSR_PMAADDR4:
		csr_write(CSR_PMAADDR4, val);
		break;
	case CSR_PMAADDR5:
		csr_write(CSR_PMAADDR5, val);
		break;
	case CSR_PMAADDR6:
		csr_write(CSR_PMAADDR6, val);
		break;
	case CSR_PMAADDR7:
		csr_write(CSR_PMAADDR7, val);
		break;
	case CSR_PMAADDR8:
		csr_write(CSR_PMAADDR8, val);
		break;
	case CSR_PMAADDR9:
		csr_write(CSR_PMAADDR9, val);
		break;
	case CSR_PMAADDR10:
		csr_write(CSR_PMAADDR10, val);
		break;
	case CSR_PMAADDR11:
		csr_write(CSR_PMAADDR11, val);
		break;
	case CSR_PMAADDR12:
		csr_write(CSR_PMAADDR12, val);
		break;
	case CSR_PMAADDR13:
		csr_write(CSR_PMAADDR13, val);
		break;
	case CSR_PMAADDR14:
		csr_write(CSR_PMAADDR14, val);
		break;
	case CSR_PMAADDR15:
		csr_write(CSR_PMAADDR15, val);
		break;
	default:
		break;

	};
}

static bool addr_align(unsigned long addr, unsigned int size)
{
	return (generic_ffs(addr) >= generic_ffs(size));
}

static int get_index(int align)
{
	int n;

	if(align) {
		n = align_index;
		if (n >= PMA_COUNT) {
			printf("NAPOT full !!!\n");
			return -EINVAL;
		}
	}
	else {
		n = tor_index + 1;

		if (n >= PMA_TOR_COUNT) {
			printf("TOR full !!!\n");
			return -EINVAL;
		}
	}

	return n;
}

static void set_index(int n, int align)
{
	if(align) {
		n ++;
		align_index = n;
	}
	else {
		n += 2;
		tor_index = n;
	}
}



static int pma_check(unsigned long addr,unsigned int size)
{
	if((generic_hweight32(size) > 1) || (!addr_align(addr, size)))
		return 0;

	return 1;
}

static int _set_pma(int n, unsigned long addr,
	    unsigned int size, int align)
{
	int pmacfg_csr, pmacfg_shift, pmaaddr_csr;
	unsigned long cfgmask, pmacfg;
	unsigned long addrmask, pmaaddr;
	unsigned long prot = 0;
	int log2len;

	log2len = generic_ffs(size) - 1;
	if (align == 1)
		if ((log2len > __riscv_xlen) || (log2len < PMA_SHIFT))
			return -EINVAL;

	/* calculate PMA register and offset */
#if __riscv_xlen == 32
	pmacfg_csr   = CSR_PMACFG0 + (n >> 2);
	pmacfg_shift = (n & 3) << 3;
#elif __riscv_xlen == 64
	pmacfg_csr   = (CSR_PMACFG0 + (n >> 2)) & ~1;
	pmacfg_shift = (n & 7) << 3;
#else
	pmacfg_csr   = -1;
	pmacfg_shift = -1;
#endif
	pmaaddr_csr = CSR_PMAADDR0 + n;
	if (pmacfg_csr < 0 || pmacfg_shift < 0)
		return -ENOTSUPP;

	/* Transfer start address to word address */
	pmaaddr = (addr >> PMA_SHIFT);
	if (align == 1)
		prot = (log2len == PMA_SHIFT) ? PMA_ETYPE_NA4 : PMA_ETYPE_NAPOT;
	else
		prot = PMA_ETYPE_TOR;

	cfgmask = ~(0xff << pmacfg_shift);
	pmacfg	= (csr_read_num(pmacfg_csr) & cfgmask);
	pmacfg |= ((prot << pmacfg_shift) & ~cfgmask);
	pmacfg |= (MTYPE_MEM_NONCACHE_BUF<< (pmacfg_shift + PMA_MTYPE_OFF));
	csr_write_num(pmacfg_csr, pmacfg);
	pmacfg = csr_read_num(pmacfg_csr);

	if (align == 1) {
		if (log2len == __riscv_xlen)
			pmaaddr = -1UL;
		else {
			addrmask = (1UL << (log2len - PMA_SHIFT)) - 1;
			pmaaddr  &= (~addrmask);
			pmaaddr |= (addrmask >> 1);
		}
		csr_write_num(pmaaddr_csr, pmaaddr);
		pmaaddr = csr_read_num(pmaaddr_csr);
	}
	else {
		csr_write_num(pmaaddr_csr - 1, pmaaddr);
		csr_write_num(pmaaddr_csr, pmaaddr + (size >> PMA_SHIFT));
		pmaaddr = csr_read_num(pmaaddr_csr - 1);
		pmaaddr = csr_read_num(pmaaddr_csr);
	}
	set_index(n, align);

	return 0;

}

int pma_set(unsigned long addr, unsigned int size)
{
	int align;
	int n;

	size = __roundup_pow_of_two(size);
	align = pma_check(addr, size);
	n = get_index(align);
	if (n < 0)
		return n;

	return _set_pma(n, addr, size, align);
}
