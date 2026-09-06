/*
 * Arch specific code for Ralink based boards
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include "config.h"

#define READREG(r)		*(volatile unsigned int *)(r)
#define WRITEREG(r,v)		*(volatile unsigned int *)(r) = v

#define KSEG1ADDR(_x)		(((_x) & 0x1fffffff) | 0xa0000000)

/* The MT7621 palmbus lives at 0x1e000000 (KSEG1 0xbe000000) while the older
 * Ralink SoCs use 0x10000000 (KSEG1 0xb0000000). loader_main() prints
 * unconditionally, so a wrong base means reading an unmapped address and
 * hanging before the kernel is ever started. Can be overridden from the
 * image Makefile with -DUART_BASE=. */
#ifndef UART_BASE
#ifdef CONFIG_SOC_MT7621
#define UART_BASE		0xbe000c00
#elif defined(CONFIG_SOC_RT288X)
#define UART_BASE		0xb0300c00
#else
#define UART_BASE		0xb0000c00
#endif
#endif

#define UART_TX			1
#define UART_LSR		7

#define UART_LSR_THRE		0x20

#define UART_READ(r)		READREG(UART_BASE + 4 * (r))
#define UART_WRITE(r,v)		WRITEREG(UART_BASE + 4 * (r), (v))

/* Bounded wait: a UART that never reports THRE must not stop the loader from
 * starting the kernel. */
static void uart_wait_tx(void)
{
	unsigned int i;

	for (i = 0; i < 0x10000; i++)
		if (UART_READ(UART_LSR) & UART_LSR_THRE)
			return;
}

void board_putc(int ch)
{
	uart_wait_tx();
	UART_WRITE(UART_TX, ch);
	uart_wait_tx();
}

void board_init(void)
{
}
