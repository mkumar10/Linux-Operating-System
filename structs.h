#ifndef _STRUCTS_H
#define _STRUCTS_H
#include "types.h"

// Struct Dependency Labels

#define FILE_NAME_CHARMAX 32
#define RESERVES 6
#define KB_1 1024
#define MAX_FD 8
#define LINE_BUFFER_SIZE    128

//define all your structures
//----------------------------------------------------------------------------------------------
// File System Structures - file_sys.c
// A structure for the directory entry
typedef struct dir_entry{
uint8_t filename[FILE_NAME_CHARMAX];
uint32_t file_type;
uint32_t inode_num;
uint32_t reserve[RESERVES];
} dentry_t;

// A structure for the inode
typedef struct inode{
uint32_t val[KB_1];
} inode_t;


//----------------------------------------------------------------------------------------------
// File Descriptor, PCB structures - file_desc.c, syscalls.c
struct __attribute__((packed)) fops
{
	int32_t (*open)(const uint8_t * );
	int32_t (*read)(int32_t , void * , int32_t );
	int32_t (*write)(int32_t , const void *, int32_t);
	int32_t (*close)(int32_t);
};

typedef struct __attribute__((packed)) file_descriptor{
	struct fops * fops_ptr;
	uint32_t inode_num;
	uint32_t pos;
	uint32_t flags;
} fd_t;

typedef struct __attribute__((packed)) PCB{
	uint32_t p_id;
	uint32_t parent_p_id;

	struct PCB * parent;
	struct PCB * child;
	fd_t fd_A[MAX_FD];

	uint8_t args[LINE_BUFFER_SIZE];

	uint32_t ESP;
	uint32_t EBP;
	uint32_t ESP_S;
	uint32_t EBP_S;
	uint32_t ESP0;

	uint32_t prog_entry;
	uint32_t fd_total_open;
	uint32_t shell_flag;
	uint32_t term_num;

} PCB_t;
//----------------------------------------------------------------------------------------------

// Data structure for command history.
typedef struct command_hist{
	uint8_t command[LINE_BUFFER_SIZE];
	struct command_hist * next;
	struct command_hist * prev;
}command_hist_t;

#endif
