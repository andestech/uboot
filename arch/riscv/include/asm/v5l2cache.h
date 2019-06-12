/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef _ASM_V5_L2CACHE_H
#define _ASM_V5_L2CACHE_H

struct l2cache {
	volatile u64	configure;
	volatile u64	control;
	volatile u64	hpm0;
	volatile u64	hpm1;
	volatile u64	hpm2;
	volatile u64	hpm3;
	volatile u64	error_status;
	volatile u64	ecc_error;
	volatile u64	cctl_command0;
	volatile u64	cctl_access_line0;
	volatile u64	cctl_command1;
	volatile u64	cctl_access_line1;
	volatile u64	cctl_command2;
	volatile u64	cctl_access_line2;
	volatile u64	cctl_command3;
	volatile u64	cctl_access_line4;
	volatile u64	cctl_status;
};

/* Control Register */
#define L2_ENABLE	0x1
/* prefetch */
#define IPREPETCH_OFF	3
#define DPREPETCH_OFF	5
#define IPREPETCH_MSK	(3 << IPREPETCH_OFF)
#define DPREPETCH_MSK	(3 << DPREPETCH_OFF)
/* tag ram */
#define TRAMOCTL_OFF	8
#define TRAMICTL_OFF	10
#define TRAMOCTL_MSK	(3 << TRAMOCTL_OFF)
#define TRAMICTL_MSK	BIT(TRAMICTL_OFF)
/* data ram */
#define DRAMOCTL_OFF	11
#define DRAMICTL_OFF	13
#define DRAMOCTL_MSK	(3 << DRAMOCTL_OFF)
#define DRAMICTL_MSK	BIT(DRAMICTL_OFF)

/* CCTL Command Register */
#define CCTL_CMD_REG(base, hart)	((ulong)(base) + 0x40 + (hart) * 0x10)
#define L2_WBINVAL_ALL	0x12

/* CCTL Status Register */
#define CCTL_STATUS_MSK(hart)		(0xf << ((hart) * 4))
#define CCTL_STATUS_IDLE(hart)		(0 << ((hart) * 4))
#define CCTL_STATUS_PROCESS(hart)	(1 << ((hart) * 4))
#define CCTL_STATUS_ILLEGAL(hart)	(2 << ((hart) * 4))

void v5l2_enable(void);
void v5l2_disable(void);

#endif /* _ASM_V5_L2CACHE_H */
