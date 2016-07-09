/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "idt_handlers.h"
#include "PS_2.h"
#include "rtc_driver.h"
#include "paging.h"
#include "file_sys.h"
#include "syscalls.h"
#include "terminal.h"
#include "file_desc.h"
#include "syscalls_linkage.h"
#include "syscaller.h"
#include "globals.h"
#include "schedule.h"
#include "pit.h"
#include "interrupt_handler.h"
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)	((flags) & (1 << (bit)))

// Function interface below
void fill_idt();
void _fill_idt_exceptions();
void _fill_idt_interrupts_sys();

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	fill_idt();
	lidt(idt_desc_ptr);	// Load IDT Pointer
	//Init the PIC
	i8259_init();
	pit_init();
	init_keyboard();
	rtc_init();
	terminal_open((uint8_t*)1);
	init_paging();


	/* initializing file systems */
	module_t * boot_fs = (module_t*)mbi->mods_addr;
	init_fs(boot_fs->mod_start);
	/* Done initializing fs */


	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	/* Enable interrupts */
	// printf("Enabling Interrupts\n");
	sti();

	// uint32_t bmap_val = 10;
	// char bitmap_temp[6];
    // itoa(bmap_val , bitmap_temp, 2);
    // // printf("BMAP VALUE - %d\n", bmap_val);
	// printf("%s\n", bitmap_temp);
    // char bitmap[6];
    // uint32_t l = strlen((uint8_t*)bitmap_temp);
    // uint32_t temp_idx;
    // for (temp_idx=0; temp_idx<5; temp_idx++){
    //     if (temp_idx<(5-l))
    //         bitmap[temp_idx] = '0';
    //     else
    //         bitmap[temp_idx] = bitmap_temp[temp_idx+l-5];
    // }
    // bitmap[5] = '\0';
    // printf("%s\n", bitmap);

	// Initialization for the process
	init_terminals();
	execute_func((uint8_t *)"shell");


	// Spin (nicely, so we don't chew up cycles)
	asm volatile(".1: hlt; jmp .1;");
}

/*
 * fill_idt
 *   DESCRIPTION: Complete the IDT table for OS.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in the IDT table.
 */
void fill_idt(){
/* Fill in the IDT descriptor table */
	_fill_idt_exceptions();

	_fill_idt_interrupts_sys();


}

/*
 * _fill_idt_exceptions
 *   DESCRIPTION: Fill in all the exception entries in the IDT.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in the IDT table.
 */
void _fill_idt_exceptions(){
	idt_desc_t idt_temp;
	// uint32_t i;
	// Exception and interrupts use interrupt gate
	idt_temp.seg_selector = KERNEL_CS;
	idt_temp.size = 1;		// Indicates 32 Bits
	idt_temp.dpl = 0x0;		// Kernel level privilege.
	idt_temp.present = 1;	// Mark as present
	idt_temp.reserved4 = 0;	// Reserved.
	idt_temp.reserved3 = 0;
	idt_temp.reserved2 = 1;
	idt_temp.reserved1 = 1;
	idt_temp.reserved0 = 0;

	SET_IDT_ENTRY(idt_temp, &exception_0);
	idt[0] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_1);
	idt[1] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_2);
	idt[2] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_3);
	idt[3] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_4);
	idt[4] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_5);
	idt[5] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_6);
	idt[6] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_7);
	idt[7] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_8);
	idt[8] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_9);
	idt[9] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_10);
	idt[10] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_11);
	idt[11] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_12);
	idt[12] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_13);
	idt[13] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_14);
	idt[14] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_15);
	idt[15] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_16);
	idt[16] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_17);
	idt[17] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_18);
	idt[18] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &exception_19);
	idt[19] = idt_temp;
}

/*
 * _fill_idt_interrupts_sys
 *   DESCRIPTION: Fill in the interrupt entries in the IDT.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in the IDT table.
 */
void _fill_idt_interrupts_sys(){
	idt_desc_t idt_temp;

	// Exception and interrupts use interrupt gate
	idt_temp.seg_selector = KERNEL_CS;
	idt_temp.size = 1;					// Indicates 32 Bits
	idt_temp.dpl = 0x0;					// Kernel level privilege.
	idt_temp.present = 1;				// Mark as present
	idt_temp.reserved4 = 0;				// Reserved.
	idt_temp.reserved3 = 0;
	idt_temp.reserved2 = 1;
	idt_temp.reserved1 = 1;
	idt_temp.reserved0 = 0;

	SET_IDT_ENTRY(idt_temp, &PIT_handler);
	idt[PIT_IDT_ENTRY] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &KB_handler);
	idt[KB_IDT_ENTRY] = idt_temp;

	SET_IDT_ENTRY(idt_temp, &RTC_handler);
	idt[RTC_IDT_ENTRY] = idt_temp;

	// Using TRAP Gates
	idt_temp.dpl = 3;
	idt_temp.reserved3 = 1;
	SET_IDT_ENTRY(idt_temp, &syscall_handler);
	idt[0x80] = idt_temp;
}
