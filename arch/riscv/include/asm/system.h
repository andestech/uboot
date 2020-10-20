/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __ASM_RISCV_SYSTEM_H
#define __ASM_RISCV_SYSTEM_H

/* 2MB granularity */
#define MMU_SECTION_SHIFT	21
#define MMU_SECTION_SIZE	(1 << MMU_SECTION_SHIFT)

#ifdef CONFIG_SYS_NONCACHED_MEMORY
int noncached_init(void);
phys_addr_t noncached_alloc(size_t size, size_t align);
#endif /* CONFIG_SYS_NONCACHED_MEMORY */

/*
 * Interrupt configuring macros.
 *
 * TODO
 *
 */

#endif	/* __ASM_RISCV_SYSTEM_H */
