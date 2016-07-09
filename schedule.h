#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "globals.h"
#include "terminal.h"
#include "syscalls.h"
#include "lib.h"

// #include "file_desc.h"
volatile static uint32_t switch_lock;

void init_terminals();
void clear_terminals();
int32_t switch_terminals(uint32_t dest_term);
// Helper Functions - Switch Terminals
// void inline do_switch(uint32_t dest_term);
void inline switch_vidmem(uint32_t dest_term);
void inline switch_vidmap(uint32_t dest_term);
void schedule(uint32_t new_term);

#endif
