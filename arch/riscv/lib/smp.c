// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <linux/io.h>
#include <fdtdec.h>
#include <asm/encoding.h>
#include <asm/csr.h>

DECLARE_GLOBAL_DATA_PTR;

//static int entry_point = 1;
long disabled_hart_mask;
extern void send_ipi(int c);
extern void hart_claim(ulong i);
extern void hart_complete(ulong i);

void get_arg(int hartid)
{
	void (*app)(ulong hart, ulong arg1);

	app = (void (*)(ulong, ulong))gd->arch.ipi.arg0;
	app(hartid, gd->arch.ipi.arg1);
}

void put_arg(ulong arg0, ulong arg1, ulong arg2)
{
	int i;
	void *addr;
	gd_t *pgd;

	for(i=1;i<gd->arch.hart_num;i++)
	{
		addr = (void *)gd->old_gd + ((1<<RISCV_PGSHIFT)*i);
		pgd = (gd_t *)addr;
		pgd->arch.ipi.arg0 = arg0;
		pgd->arch.ipi.arg1 = arg1;
		pgd->arch.ipi.arg2 = arg2;
	}
}

void mstatus_init(void)
{
	// Enable software interrupts
	write_csr(mie, MIP_MSIP);
}

void plic_init(void)
{
	mstatus_init();
}

static void hart_plic_init(void)
{
	// clear pending interrupts
	write_csr(mip, 0);
}

void wake_harts(int c)
{
	send_ipi(c);
}

void init_first_hart(int hartid)
{
	plic_init();
	hart_plic_init();
}


void init_other_hart(int hartid)
{
	// ipi wake up wfi under MSTATUS_MIE off, i.e. no software trap raised.
	// plicsw pending bit has to be clear here
	plic_init();
	hart_claim(hartid);
	hart_complete(hartid);
	hart_plic_init();
	get_arg(hartid);
}


