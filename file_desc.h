#ifndef _FILE_DESC_H
#define _FILE_DESC_H

#include "file_sys.h"
#include "lib.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "rtc_driver.h"
#include "globals.h"

#define _128MB 0x8000000
#define PROG_OFF 0x00048000
#define _8MB (2*MEM4MB)
#define _8KB (2*HEX4KB)
#define IN_USE 1
#define MAX_ARGS 3

#define _4B 4
#define PD_ENTRY_PROG 32
#define PROG_ENTRY_OFFSET 24
#define PAGESIZE4MB_PROG 0x87

// --------------------------------------------------------------------
// All PCB Functions go in here
void PCB_init(PCB_t * pcb_ptr, uint32_t p_id, uint8_t * str_args, dentry_t * file_entry);
PCB_t* pid_to_PCB_addr(uint32_t p_id);
//-----------------------------------------------------------
// Helper PCB Functions for HALT
void inline get_parent_PCB();
void inline swap_pages(uint32_t p_id_new);
void inline flush_tlb();
void inline save_esp_ebp(PCB_t* pcb_ptr);

// --------------------------------------------------------------------
void fd_A_init(fd_t* fd_A);


// --------------------------------------------------------------------
int32_t invalid_func();

#endif
