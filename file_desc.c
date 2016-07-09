#include "file_desc.h"

// STDIN
static struct fops stdin_fops = {
	.open = invalid_func,
	.read = terminal_read,
	.write =  invalid_func,
	.close = invalid_func
};

// STDOUT
static struct fops stdout_fops = {
	.open = invalid_func,
	.read = invalid_func,
	.write = terminal_write,
	.close = invalid_func
};


//------------------------------------------------------------------------------
// PCB Functions

void PCB_init(PCB_t * pcb_ptr, uint32_t p_id, uint8_t * str_args, dentry_t * file_entry){

	if(curr_pcb_ptr != NULL && next_p_id[curr_term] != 0)
	{
		pcb_ptr->p_id = next_p_id[curr_term];
		pcb_ptr->parent_p_id = next_p_id[curr_term] - 1;

		// Child of current Process is this new process
		curr_pcb_ptr->child = pcb_ptr;

		pcb_ptr->parent = curr_pcb_ptr;
		pcb_ptr->child = NULL;
	}
	else
	{
		pcb_ptr->p_id = 0;
		pcb_ptr->parent_p_id = 0;
		pcb_ptr->parent = NULL;
		pcb_ptr->child = NULL;
	}

	// Initialize the FD Array
	fd_A_init(pcb_ptr->fd_A);

	// Save the string arguments passed in.
	strcpy((int8_t*)pcb_ptr->args, (int8_t*)str_args);
	// Save the terminal the program is executed on.
	pcb_ptr->term_num = curr_term;

	// pcb_ptr->ESP0 = tss.esp0;

	// Setup the program's entry point
	read_data(file_entry->inode_num, PROG_ENTRY_OFFSET, (uint8_t *) &(pcb_ptr->prog_entry), _4B);

	pcb_ptr->fd_total_open = 1;
}

//-----------------------------------------------------------
// Functions for get ESP and EBP
void inline save_esp_ebp(PCB_t* pcb_ptr){
	// Save Parent's ESP, EBP, ESP0
	uint32_t flags;
	cli_and_save(flags);
	asm __volatile__("movl %%esp, %0" :"=r" (pcb_ptr->ESP));
	asm __volatile__("movl %%ebp, %0" :"=r" (pcb_ptr->EBP));
	sti();
	restore_flags(flags);
}

//-----------------------------------------------------------
// Functions for HALT
void inline get_parent_PCB()
{
	if(curr_pcb_ptr->parent != NULL)
	{
		swap_pages(curr_pcb_ptr->parent_p_id);
		// Exchange with parent PCB
		curr_pcb_ptr = curr_pcb_ptr->parent;
		active_progs[curr_term] = curr_pcb_ptr;
	}
}

void inline swap_pages(uint32_t p_id_new)
{
	page_directory[PD_ENTRY_PROG] = (MEM4MB*((p_id_new + curr_term * MAX_PROGS) + 2) | PAGESIZE4MB_PROG);
	flush_tlb();
}

void inline flush_tlb()
{
	// Flush TLB - Reload the same CR 3 value
	uint32_t flags;
	cli_and_save(flags);
	asm __volatile__("movl %cr3, %eax; \
					  movl %eax, %cr3");
	sti();
	restore_flags(flags);

}
//-----------------------------------------------------------

// BUG FREE - CHECK - BA
PCB_t* pid_to_PCB_addr(uint32_t p_id){
	return (PCB_t *)(_8MB - (_8KB * ((p_id + curr_term * MAX_PROGS)+ 1)));
}

//------------------------------------------------------------------------------
// File Descriptor Functions

// Initialize the FD Array
void fd_A_init(fd_t* fd_A){
	// Setup the First 2 FDs
	fd_A[0].fops_ptr = &stdin_fops;		// STDIN FOPS Table
	fd_A[0].inode_num = -1;
	fd_A[0].pos = 0;
	fd_A[0].flags = !IN_USE;

	fd_A[1].fops_ptr = &stdout_fops;	// STDOUT FOPS Table
	fd_A[1].inode_num = -1;
	fd_A[1].pos = 0;
	fd_A[1].flags = !IN_USE;

	uint32_t i;
	for(i=2; i<MAX_FD; i++){
		fd_A[i].fops_ptr = NULL;
		fd_A[i].inode_num = -1;
		fd_A[i].pos = 0;
		fd_A[i].flags = !IN_USE;
	}
}


//--------------------------------------------------------------------------------
// Invalid Call Function for use everywhere
int32_t invalid_func()
{
	return -1;
}
