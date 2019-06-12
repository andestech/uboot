/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#ifdef CONFIG_SMP
struct ipi_data {
	ulong arg0;
	ulong arg1;
	ulong arg2;
};

typedef struct {
	int hart_id;
	int source_id;
	volatile uint32_t* enable[2];
	volatile uint32_t* pending[2];
	volatile uint32_t* claim[2];
	int hart_num;
} plic_sw_t;
#endif
/* Architecture-specific global data */
struct arch_global_data {
	long boot_hart;		/* boot hart id */
	long current_hart;	/* current hart id */
	long hart_num;		/* hart number */
	long boot_dtb;		/* boot dtb addr */
	long boot_app;		/* boot app addr */
#ifdef CONFIG_SIFIVE_CLINT
	void __iomem *clint;	/* clint base address */
#endif
#ifdef CONFIG_NDS_PLIC
	void __iomem *plic;	/* plic base address */
#endif
#ifdef CONFIG_V5L2_CACHE
	void __iomem *v5l2;	/* v5l2 base address */
#endif
#ifdef CONFIG_SMP
	struct ipi_data ipi;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm ("gp")

#endif /* __ASM_GBL_DATA_H */
