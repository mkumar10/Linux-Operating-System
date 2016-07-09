#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "file_sys.h"
#include "lib.h"
#include "terminal.h"
#include "file_desc.h"
#include "paging.h"
#include "x86_desc.h"
#include "globals.h"
#include "rtc_driver.h"

#define STACK_PTR_OFF 4
#define REGULAR_FILE_TYPE 2

int32_t halt_func(uint8_t status);
int32_t execute_func(const uint8_t * command);
int32_t read_func(int32_t fd, void * buf, int32_t nbytes);
int32_t write_func(int32_t fd, const void * buf, int32_t nbytes);
int32_t open_func(const uint8_t * filename);
int32_t close_func(uint32_t fd);
int32_t getargs_func(uint8_t * buf, int32_t nbytes);
int32_t vidmap_func(uint8_t ** screen_start);
int32_t set_handler_func(int32_t signum, void * handler_address);
int32_t sigreturn_func(void);



//--------------------------------------------------------------
// Execute Helper Functions
uint8_t* pid_to_kstack_bottom(uint32_t p_id);
uint8_t * ustack_bottom();
// Make the Context Switch
void inline push_stack_iret(PCB_t* pcb_ptr, uint32_t entry_point, uint16_t DS, uint16_t CS);
// Get the arguments from the command
void get_args_fname(const uint8_t * command, uint8_t * file_name, uint8_t * args );
// Vidmap Helper Function
uint32_t vidmem_p_addr(uint32_t p_id);

#endif
