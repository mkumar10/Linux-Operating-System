#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "structs.h"

#define NUM_TERMS 3
#define MAX_PROGS 6
//----------------------------------------------------------------------
//Global Variables Section


PCB_t * curr_pcb_ptr;			// The Pointer to the Current Program's PCB

PCB_t * active_progs[NUM_TERMS];
volatile uint32_t total_progs;
volatile uint32_t curr_term;		// The one that is running
volatile uint32_t disp_term;		// The one you can seeee

int32_t next_p_id[NUM_TERMS];
volatile int32_t halt_retval[NUM_TERMS];	// The return value of the current program
//-------------------------------------------------------------
// Contains pointers to video buffers for Terminals
uint8_t * video_buffer[NUM_TERMS];

uint32_t pit_counter;

#endif
