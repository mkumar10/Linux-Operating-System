#include "syscalls.h"
//--------------------------------------------------------------------------------------------------
// FOPS Tables
// RTC File
static struct fops rtc_fops = {
	.open = rtc_open,
	.read = rtc_read,
	.write = rtc_write,
	.close = rtc_close
};

//Dir File - Temp Filler
static struct fops dir_fops = {
	.open = file_open,
	.read = read_dir,
	.write = invalid_func,
	.close = file_close
};

// Data File
static struct fops file_fops = {
	.open = file_open,
	.read = read_data_file,
	.write = file_write,
	.close = file_close
};


//--------------------------------------------------------------------------------------------------

// How to Execute Something ->
// 1. Parse the command
// 2. Check if executable
// 3. Set up paging for that program
// 4. File Loader
// 5. Allocate a new PCB
// 6. Context Switching

/*
 * execute_func
 *   DESCRIPTION: execute system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t execute_func(const uint8_t * command)
{
	uint8_t file_name[FILE_NAME_CHARMAX];
	uint8_t args[LINE_BUFFER_SIZE];
	get_args_fname(command, file_name, args);
	// Check if we can execute more programs

	if(total_progs >= MAX_PROGS)
	{
		terminal_printf("Cannot Execute more programs.\n");
		return -1;
	}
	// Check if the file exists is valid and is an executable
	dentry_t file_entry;
	if(-1 == read_dentry_by_name(file_name, &file_entry))
	{
		terminal_printf("File Not Found\n");
		return -1;
	}
	if(file_entry.file_type != REGULAR_FILE_TYPE)
	{
		terminal_printf("Not a valid File\n");
		return -1;
	}
	uint8_t exec_check[_4B];
	read_data(file_entry.inode_num, 0, exec_check, _4B);
	if(exec_check[0]!= ELF_FORMAT && exec_check[1] != 'E' && exec_check[2] != 'L' && exec_check[3] != 'F')
	{
		terminal_printf("Not an executable\n");
		return -1;
	}

	// Allocate the page for the program
	page_directory[PD_ENTRY_PROG] = (MEM4MB*((next_p_id[curr_term] + curr_term * MAX_PROGS) + 2) | PAGESIZE4MB_PROG);
	// Flush TLB Entries
	flush_tlb();

	uint8_t* page_addr = (uint8_t *)_128MB;

	// Load file from FS into Program Page
	uint32_t f_size  = get_file_size(&file_entry);
	read_data(file_entry.inode_num, 0, (page_addr + PROG_OFF), f_size);

	// terminal_printf("%x\n", *((uint8_t*)(page_addr + PROG_OFF)));		// A Debugger Line

	// Allocate PCB
	PCB_t * pcb_prog = pid_to_PCB_addr(next_p_id[curr_term]);
	// Initilize the new PCB
	PCB_init(pcb_prog, next_p_id[curr_term], args, &file_entry);
	// Set shell flag for PCB
	if (0==strncmp((int8_t*)file_name, (int8_t*)"shell", strlen((int8_t*)"shell"))){
		pcb_prog->shell_flag = 1;
	}else{
		pcb_prog->shell_flag = 0;
	}
	// terminal_printf("%d\n",curr_pcb_ptr->shell_flag);
	// Bottom of stack - Address Calculations
	uint8_t * prog_kstack_bottom = pid_to_kstack_bottom(next_p_id[curr_term]);
	uint8_t * prog_ustack_bottom = ustack_bottom();

	// Done initializing - Setup the next_p_id, curr_pcb_ptr
	curr_pcb_ptr = pcb_prog;
	next_p_id[curr_term]++;
	// Init for multiple terminals
	active_progs[curr_term] = curr_pcb_ptr;
	total_progs++;

	//-----------------------------------------------------------------------
	// Context Switch
	// Save the ESP0
	curr_pcb_ptr->ESP0 = (uint32_t) prog_kstack_bottom;
	//TSS Setup
	tss.ss0 = KERNEL_DS;
	tss.esp0 = (uint32_t) prog_kstack_bottom;

	halt_retval[curr_term] = 0;
	push_stack_iret(curr_pcb_ptr, (uint32_t)prog_ustack_bottom, (uint16_t)USER_DS, (uint16_t)USER_CS);

	// terminal_printf("No Page fault in halt\n");
	uint32_t old_retval = halt_retval[curr_term];
	halt_retval[curr_term] = 0;
	return old_retval;
}



//-------------------------------------------------------------------------------------------------
// Execute Helper Functions
/*
 * pid_to_kstack_bottom
 *   DESCRIPTION: execute system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: Address of kernel stack.
 *   SIDE EFFECTS: .
 */
uint8_t * pid_to_kstack_bottom(uint32_t p_id){
	// An offset on the stack ptr is requried
	return (uint8_t *)(_8MB - (_8KB * (p_id + curr_term * MAX_PROGS)) - STACK_PTR_OFF);
}

/*
 * ustack_bottom
 *   DESCRIPTION: execute system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: Address of user stack.
 *   SIDE EFFECTS: .
 */
uint8_t * ustack_bottom(){
	return (uint8_t *)(_128MB + MEM4MB - STACK_PTR_OFF);
}
// Used for Context Switching

/*
 * push_stack_iret
 *   DESCRIPTION: setup the stack for iret.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: .
 */
void inline push_stack_iret(PCB_t* pcb_ptr, uint32_t stack_bottom, uint16_t DS, uint16_t CS){
		uint32_t flags;
		cli_and_save(flags);
		asm __volatile__("movl %%esp, %0" :"=r" (pcb_ptr->ESP));
		asm __volatile__("movl %%ebp, %0" :"=r" (pcb_ptr->EBP));
		sti();
		restore_flags(flags);

		asm __volatile__ ("  \
            cli; \
            xorl %%eax, %%eax;\
            movw %0, %%ax; \
            movw %%ax, %%ds; \
            movw %%ax, %%es; \
            movw %%ax, %%fs; \
            movw %%ax, %%gs; \
            \
            pushl %%eax;\
            \
            movl %3, %%eax;\
            pushl %%eax; \
			\
            pushfl; \
			popl %%eax;\
			orl $0x200, %%eax ; \
			pushl %%eax;\
            \
            xorl %%eax, %%eax;\
            movw %1, %%ax;\
            pushl %%eax; \
            \
            movl %2, %%eax  ;\
            pushl %%eax; \
            iret; \
			RETURN_HERE:"
            :       // No outputs
            : "r" (DS), "r" (CS), "r" (pcb_ptr->prog_entry), "r"(stack_bottom)
            : "eax");
}
//-------------------------------------------------------------------------------------------------

/*
 * get_args_fname
 *   DESCRIPTION: get arguments.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: put the arguments to buffer.
 *   SIDE EFFECTS: .
 */
void get_args_fname(const uint8_t * command, uint8_t * file_name, uint8_t * args ){

	int32_t i = 0, j = 0;
	// Get command
	for (i = 0; (uint8_t)command[i]==' '; i++);

	while(!(command[i] == '\0' || command[i] == ' ' || command[i] == '\n' || command[i] == '\r') && j < FILE_NAME_CHARMAX)
	{
		file_name[j] = command[i];
		i++;
		j++;
	}
	if (j<FILE_NAME_CHARMAX) file_name[j] = '\0';

	for (j = 0; (uint8_t)command[i]==' '; i++);

	while(command[i] != '\0' && command[i] !='\n' && command[i] != '\r' && j < LINE_BUFFER_SIZE)
	{
		args[j] = command[i];
		i++;
		j++;
	}
	args[j] = '\0';
	return;
}

//-------------------------------------------------------------------------------------------------
// Read that calls the appropriate function
/*
 * read_func
 *   DESCRIPTION: read system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t read_func(int32_t fd, void * buf, int32_t nbytes)
{
	if(fd < 0 || fd >= MAX_FD|| curr_pcb_ptr->fd_A[fd].fops_ptr == NULL ||nbytes <= 0 || buf == NULL || curr_pcb_ptr->fd_A[fd].flags == IN_USE)
		return -1;
	int32_t (*func_read) (int32_t, void *, int32_t) = (void *)(curr_pcb_ptr->fd_A[fd]).fops_ptr->read;
	if(func_read == NULL)
		return -1;
	return func_read(fd, buf, nbytes);
}

//curr_pcb_ptr->fd_A[fd].fops_ptr == NULL - used to mark an uninitialized File Descriptor
//-------------------------------------------------------------------------------------------------
// Read that calls the appropriate function
/*
 * write_func
 *   DESCRIPTION: write_func system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t write_func(int32_t fd, const void * buf, int32_t nbytes)
{
	if(fd < 0 || fd >= MAX_FD|| nbytes <= 0 || curr_pcb_ptr->fd_A[fd].fops_ptr == NULL || buf == NULL || curr_pcb_ptr->fd_A[fd].flags == IN_USE)
		return -1;
	int32_t (*func_write) (int32_t, const void *, int32_t) = (void *)(curr_pcb_ptr->fd_A[fd]).fops_ptr->write;
	if(func_write == NULL)
		return -1;

	return func_write(fd, buf, nbytes);
}
//-------------------------------------------------------------------------------------------------
// Open the appropriate file
/*
 * open_func
 *   DESCRIPTION: open system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t open_func(const uint8_t * filename)
{
	// Check if file exists and it can be opened
	if(curr_pcb_ptr->fd_total_open >= MAX_FD  - 1)
	{
		terminal_printf("Cannot Open More Files\n");
		return -1;
	}

	dentry_t file_entry;
	if(-1 == read_dentry_by_name(filename, &file_entry))
	{
		terminal_printf("File Does not exist\n");
		return -1;
	}
	curr_pcb_ptr->fd_total_open++;
	// Find the next available fd
	uint32_t new_fd;
	int32_t i;
	for(i = 2; i<MAX_FD; i++)
	{
		if(curr_pcb_ptr->fd_A[i].fops_ptr == NULL)
		{
			new_fd = i;
			break;
		}
	}

	switch(file_entry.file_type) {
		// RTC File Type
		case 0:
		curr_pcb_ptr->fd_A[new_fd].fops_ptr = &rtc_fops;
		curr_pcb_ptr->fd_A[new_fd].inode_num = -1;
		curr_pcb_ptr->fd_A[new_fd].pos = 0;
		curr_pcb_ptr->fd_A[new_fd].flags = !IN_USE;
		break;

		// Directory File
		case 1:
		curr_pcb_ptr->fd_A[new_fd].fops_ptr = &dir_fops;
		curr_pcb_ptr->fd_A[new_fd].inode_num = -1;
		// Begin reading from the first entry
		curr_pcb_ptr->fd_A[new_fd].pos = 1;
		curr_pcb_ptr->fd_A[new_fd].flags = !IN_USE;
		break;

		// A Regular File
		case 2:
		curr_pcb_ptr->fd_A[new_fd].fops_ptr = &file_fops;
		curr_pcb_ptr->fd_A[new_fd].inode_num = file_entry.inode_num;
		curr_pcb_ptr->fd_A[new_fd].pos = 0;
		curr_pcb_ptr->fd_A[new_fd].flags = !IN_USE;
		break;
	}
	// Fix the different fops tables - Refer to Appendix B requirements

	int32_t (*func_open) (const uint8_t * ) = (void *)(curr_pcb_ptr->fd_A[new_fd]).fops_ptr->open;
	if(func_open == NULL)
	{
		terminal_printf("No Valid Function for Open Defined\n");
		return -1;
	}

	if(-1 == func_open(filename))
	{
		// terminal_printf("File Open Failed\n");
		return -1;
	}
	return new_fd;
}
//-------------------------------------------------------------------------------------------------
// Close the appropriate file
// This function assumest that you first close the last fd opened.
// For ex. once all 7 are open, new fds can be closed only one by one starting from the last fd.
/*
 * Close
 *   DESCRIPTION: close system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t close_func(uint32_t fd)
{
	if(fd <= 1 || fd >= MAX_FD || curr_pcb_ptr->fd_A[fd].fops_ptr == NULL)
	{
		return -1;
	}
	int32_t (*func_close)(int32_t) = (void *)(curr_pcb_ptr->fd_A[fd]).fops_ptr->close;

	if(func_close == NULL)
	{
		terminal_printf("No Valid Function for Close Defined\n");
		return -1;
	}

	// Clear the fd array entry
	curr_pcb_ptr->fd_A[fd].fops_ptr = NULL;
	curr_pcb_ptr->fd_A[fd].inode_num = 0;
	curr_pcb_ptr->fd_A[fd].pos = 0;
	curr_pcb_ptr->fd_A[fd].flags = 0;

	curr_pcb_ptr->fd_total_open--;
	return func_close(fd);
}
//-----------------------------------------------------------------------------------------------

// Get the saved arguments
/*
 * getargs_func
 *   DESCRIPTION: getargs_func system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t getargs_func(uint8_t * buf, int32_t nbytes)
{
	// If more arguments that can fit in buffer then return -1 - Check for invalid inputs
	if(buf == NULL)
	{
		return -1;
	}
	strcpy((int8_t *)buf, (int8_t *)curr_pcb_ptr->args);
	return 0;
}
//-----------------------------------------------------------------------------------------------
/*
 * vidmap_func
 *   DESCRIPTION: vidmap system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: change video memory.
 */
int32_t vidmap_func(uint8_t ** screen_start)
{
	if(screen_start >(uint8_t **) (_128MB + MEM4MB) || screen_start < (uint8_t **)_128MB)
		return -1;


	// Change the video memory mapping in physical mem
	// page_table[VidMemidx] = vidmem_p_addr(curr_pcb_ptr->p_id);
	//----------------------------------------------
	video_table[curr_term] |= 4;
	(*screen_start) = (uint8_t *)(_128MB + MEM4MB + curr_term * SIZE4KB);
	// Give user the access ...
	// page_directory[0] |= 4;
	// page_table[VidMemidx + curr_term + 1] |= 4;

	flush_tlb();
	return 0;
}

// Not in use any more. (For extra point)
uint32_t vidmem_p_addr(uint32_t p_id)
{
	return video_table[p_id];
}

//-----------------------------------------------------------------------------------------------
/*
 * halt
 *   DESCRIPTION: halt the Program.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t halt_func(uint8_t status)
{
	// Added Cleanup Code for multiple terminals case
	total_progs--;
	// Restoring next_p_id value
	next_p_id[curr_term]--;
	// current PCB pointer
	while(curr_pcb_ptr->parent == NULL)
	{
		terminal_printf("\nRestart the shell.\n");
		execute_func((uint8_t*)"shell");
	}
	// terminal_printf("\nExited process %d\n", next_p_id[curr_term]);
	uint32_t ESP_save = curr_pcb_ptr->ESP, EBP_save = curr_pcb_ptr->EBP;

	uint32_t flags;
	cli_and_save(flags);
	get_parent_PCB();   // replacing pcb_pointer with its parent pointer to halt the process
	rtc_default_rate();


	curr_pcb_ptr->child = NULL;
	tss.esp0 = curr_pcb_ptr->ESP0;
 	// restore esp and ebp
	asm __volatile__("movl  %0, %%esp"  : :"r" (ESP_save));
	asm __volatile__("movl  %0, %%ebp"  : :"r" (EBP_save));
	sti();
	restore_flags(flags);

	asm volatile(" jmp RETURN_HERE");
	return 0;
}

//-----------------------------------------------------------------------------------------------
/*
 * set_handler_func
 *   DESCRIPTION: set_handler_func system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t set_handler_func(int32_t signum, void * handler_address)
{
	return -1;
}
/*
 * sigreturn_func
 *   DESCRIPTION: sigreturn_func system call.
 *   INPUTS: command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fails. 0 on success.
 *   SIDE EFFECTS: .
 */
int32_t sigreturn_func(void)
{
	return -1;
}
