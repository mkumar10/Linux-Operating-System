#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"
#include "x86_desc.h"
#include "globals.h"
#include "terminal.h"

#define SIZE4KB 4096
#define SIZE1KB 1024
#define VidMemidx 184
#define MEM4MB 0x400000
#define HEX4KB 0x1000
#define PAGESIZE4MB 0x83
#define VM_VIDEO_IDX 33

uint32_t page_directory[SIZE1KB] __attribute__((aligned(SIZE4KB)));		//  declaring page derectory and page table

uint32_t page_table[SIZE1KB] __attribute__((aligned(SIZE4KB)));
//-----------------------------------
uint32_t video_table[SIZE1KB] __attribute__((aligned(SIZE4KB)));
//-----------------------------------

extern void enable_paging();

extern void init_paging();
#endif
