#include "schedule.h"

#define do_switch(dest_term)  \
do { 															\
	/* // Save my ESP, EBP */ \
	asm __volatile__("movl %%esp, %0" :"=r" (curr_pcb_ptr->ESP_S)); \
	asm __volatile__("movl %%ebp, %0" :"=r" (curr_pcb_ptr->EBP_S)); \
 											\
	active_progs[curr_term] = curr_pcb_ptr; \
	curr_pcb_ptr = active_progs[dest_term]; \
 											\
	curr_term = dest_term;	 \
 	/*sti();*/							\
	if(curr_pcb_ptr == NULL) \
	{ 									\
		cli();								\
		switch_lock = 0;					\
		execute_func((uint8_t *)"shell"); \
		/*// terminal_printf("\nStarting terminal no. %d\n", dest_term+1);*/ \
	} \
	/*if(curr_pcb_ptr->shell_flag)*/\
		terminal_update_cursor();\
	/*//------------------------------------------------------------*/\
	/* // Switch TSS */\
	tss.ss0 = KERNEL_DS;\
	tss.esp0 = curr_pcb_ptr->ESP0;\
	swap_pages(curr_pcb_ptr->p_id);\
	/* // Check whether in user or kernel mode currently */\
	/* // Switch to kernel mode */\
	/* // Change ESP, EBP */\
 	/* // restore esp and ebp */\
	/* // cli_and_save(flags); */\
	asm __volatile__("movl  %0, %%esp"  : :"r" (curr_pcb_ptr->ESP_S));\
	asm __volatile__("movl  %0, %%ebp"  : :"r" (curr_pcb_ptr->EBP_S));\
	/* // sti(); */\
	/* // restore_flags(flags); */\
}while(0);

// Using lazy approach to start new terminals
void init_terminals()
{
	uint32_t i;
	for (i = 0; i < NUM_TERMS; ++i)
	{
		curr_term = i;
		next_p_id[curr_term] = 0;
		active_progs[curr_term] = NULL;
	}
	clear_terminals();
	curr_pcb_ptr = NULL;
	curr_term = 0;
	disp_term = 0;
	total_progs = 0;

	switch_lock = 0;
	return;
}

void clear_terminals()
{
	uint32_t i, j;
	uint8_t *video_mem = NULL;
	for(j = 0; j<NUM_TERMS; j++)
	{
		video_mem = video_buffer[j];
		for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
	        *(video_mem + (i << 1)) = ' ';
	        *(video_mem + (i << 1) + 1) = ATTRIB;
	    }
	}
}


// Switch Terminals -
// 1. Change TSS
// 2. Setup/Change Paging
// 3. Cleanup
// 4. Save ESP/EBP
// 5. Return
/*
int32_t switch_terminals(uint32_t dest_term)
{
	if(disp_term == dest_term)
		return -1;

	if( 0 > dest_term || dest_term >= NUM_TERMS)
		return -1;

	// uint32_t flags;
	// cli_and_save(flags);
	if(!switch_lock)
	{
		switch_lock = 1;
		// cli();
		switch_vidmem(dest_term);
		switch_vidmap(dest_term);
		disp_term = dest_term;
		// sti();
		// restore_flags(flags);
		do_switch(dest_term);
		switch_lock = 0;
	}
	return 0;
}
*/
// V.2.0
int32_t switch_terminals(uint32_t dest_term)
{
	if(disp_term == dest_term)
		return -1;

	if( 0 > dest_term || dest_term >= NUM_TERMS)
		return -1;

	if(!switch_lock)
	{
		switch_lock = 1;
		switch_vidmem(dest_term);
		switch_vidmap(dest_term);
		disp_term = dest_term;

		if(active_progs[dest_term] == NULL)
		{
			do_switch(dest_term);
		}
		switch_lock = 0;
	}

	return 0;
}


void inline switch_vidmem(uint32_t dest_term)
{
	// copy from VM to curr_term buffer
	memcpy(video_buffer[disp_term], (uint8_t*)VIDEO, NUM_ROWS*NUM_COLS*2);
	// copy from dest_term buffer to VM
	memcpy((uint8_t*)VIDEO, video_buffer[dest_term], NUM_ROWS*NUM_COLS*2);
	return;
}

//------------------------------------------------------------------------------
// pit_handler -> calls schedule()
// pit init
/*
void schedule(uint32_t new_term)
{
	// uint32_t flags;
	// cli_and_save(flags);
	if(active_progs[new_term] != NULL)
	{
		if(!switch_lock)
		{
			switch_lock = 1;
			do_switch(new_term);
			switch_lock = 0;
		}
	}
}*/
// V.2.0
void schedule(uint32_t new_term)
{
	// uint32_t flags;
	// cli_and_save(flags);
	if(active_progs[new_term] != NULL)
	{
		if(!switch_lock)
		{
			switch_lock = 1;
			do_switch(new_term);
			switch_lock = 0;
		}
	}
}

void inline switch_vidmap(uint32_t dest_term)
{
	// Change disp_term to Buffer
	video_table[disp_term] = (uint32_t)video_buffer[disp_term] | 0x7;
	// Change dest_term to VM
	video_table[dest_term] = (VIDEO) | 0x7;
	flush_tlb();
}
