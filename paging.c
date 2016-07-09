#include "paging.h"

/*
 * enable_paging
 *   DESCRIPTION: Enable paging by setting CR0.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Turn on paging and change cr0.
 */
void enable_paging()
{
    // enabling paging
    // Reference : OS Dev
    // enables 4 MB pages
    // setting the paging bit of CR0
    asm volatile ("                   \
        movl %%cr4, %%eax   \n\
        orl $0x00000010, %%eax    \n\
        movl %%eax, %%cr4     \n\
        movl %%cr0, %%eax   \n\
        orl $0x80000000, %%eax  \n\
        movl %%eax, %%cr0 "
        : // no outputs
        : // no inputs
        : "%eax"
        ); // do We have to enable page.
}



/*
 * init_paging
 *   DESCRIPTION: Initialize all the paging tables.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Turn on paging.
 */
void init_paging()
{

	uint32_t i;
 	for(i=0; i < SIZE1KB; i++)	{			// initializing the directory to read/write, not present, and supervisor
        page_directory[i] = 0x2; // initializing the page table to read/write, not present, and supervisor for 0-4MB of memory
        page_table[i] = ((i * HEX4KB) | 3);   // settng the index of each page in the page table and setting it as available for read/write
        //-----------------------------------------
        // video_table[i] = (( HEX4KB*(SIZE1KB - 1) + (i* HEX4KB) )| 3);  // Allocate Pages for the Video Table
        video_table[i] = 0;          // Set to read/write, not present and supervisor access 
    }

	page_table[VidMemidx]= (page_table[VidMemidx] | 0x1); 			// mapping the video memory to present.  Or with one to set the present bit

    for(i=1; i<=NUM_TERMS; i++){
        page_table[VidMemidx+i] |= 0x1;
        video_buffer[i - 1] = (uint8_t*)(page_table[VidMemidx+i] & 0xFFFFF000);
        video_table[i - 1] = (VIDEO) | 0x7;
    }

    // kernel_vid_mem = page_table[VidMemidx];
	page_directory[0] = (((unsigned int)page_table) | 3);          // setting page_directory[0] for reading/writing and present

	page_directory[1] = (MEM4MB | PAGESIZE4MB); 				// paging for 4-8MB  to physical memory.  i.e setting the size bit

    // Added Video Memory Support Here ------------
    // page_directory[SIZE1KB - 1] = (((unsigned int)video_table) | 7);
    //-----------------------------------
    page_directory[VM_VIDEO_IDX] = (((unsigned int)video_table) | 7);
    // loading cr3 with the address of the page directory
    asm volatile ("               \
        movl %0, %%eax   \n\
        movl %%eax, %%cr3"
        : /* no outputs */
        : "r"(page_directory)
        :"%eax"
        );

    enable_paging();  // calling enabling paging
}
