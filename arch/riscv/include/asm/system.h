/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __ASM_RISCV_SYSTEM_H
#define __ASM_RISCV_SYSTEM_H

struct event;
/* 2MB granularity */
#define MMU_SECTION_SHIFT	21
#define MMU_SECTION_SIZE	(1 << MMU_SECTION_SHIFT)

#ifdef CONFIG_SYS_NONCACHED_MEMORY
int noncached_init(void);
phys_addr_t noncached_alloc(size_t size, size_t align);
#endif

/*
 * Interrupt configuring macros.
 *
 * TODO
 *
 */

/* Hook to set up the CPU (called from SPL too) */
int riscv_cpu_setup(void *ctx, struct event *event);

#endif	/* __ASM_RISCV_SYSTEM_H */
