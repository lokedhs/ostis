#ifndef DMA_H
#define DMA_H

#include "common.h"
#include "cpu.h"

void dma_set_error();
void dma_clr_error();
void dma_inc_address(LONG);
void dma_init();
void dma_do_interrupts(struct cpu *);

#endif
