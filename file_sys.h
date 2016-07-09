#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "lib.h"
#include "terminal.h"
#include "x86_desc.h"
#include "globals.h"

#define BYTES_PER_BLOCK 0x1000
#define STAT_BYTES  12
#define STAT_SIZE 64
#define NUM_DENTRY 62
#define ELF_FORMAT	0x7f


/*
	init_fs function: initializes the file system
 */

extern void init_fs(uint32_t boot_addr);
extern int32_t read_dentry_by_name(const uint8_t *fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry);
// The actual read_data from file wrapper function
extern int32_t read_data_file(int32_t fd,  void* buf, int32_t length);
extern int32_t read_data(uint32_t inode, uint32_t offset,  uint8_t * buf, uint32_t length);

// The read_dir(next filename) wrapper function
extern int32_t read_dir(int32_t fd,  void* buf, int32_t length);
extern int32_t read_file_dir();

extern inode_t * calculate_inode_start();
extern dentry_t * calculate_dentries_start();
//-------------------------------------------------
// Test Functions for the file systems
extern void print_file_size();
extern void print_file_names();
extern void print_file(char * filename, uint32_t offset, uint32_t length);
extern uint32_t get_file_size(dentry_t * Mydentry);
extern int32_t file_open();
extern int32_t file_close ();
extern int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
#endif
