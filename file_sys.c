#include "file_sys.h"

uint32_t fs_stats[3];
uint32_t bootaddr;			  // Boot address


/*
 * init_fs
 *   DESCRIPTION: Initialize the file system.
 *   INPUTS: uint32_t boot_addr: address of boot block
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: updates staic boot addr pointer and fills stats array
 */

void init_fs(uint32_t boot_addr)
{
	bootaddr = boot_addr;
	memcpy(&fs_stats, (void*)boot_addr, STAT_BYTES);

}

/*
 * calculate_dentries_start
 *   DESCRIPTION: Calculates stating address of  directory entries
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to dentry start
 *   SIDE EFFECTS: none
 */

dentry_t * calculate_dentries_start()
{
	return (dentry_t *)(bootaddr + STAT_SIZE);
}

/*
 * calculate_inode_start
 *   DESCRIPTION: Calculates stating address of inode block
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to inode starting block
 *   SIDE EFFECTS: none
 */
inode_t * calculate_inode_start()
{
	return  (inode_t*)(bootaddr + BYTES_PER_BLOCK);
}


/*
 * read_dentry_by_name
 *   DESCRIPTION: fills the passed in dentry structure with the data for the directory of the same name
 *   INPUTS: uint8_t * fname, dentry_t * dentry: file name and dentry structure
 *   OUTPUTS: none
 *   RETURN VALUE:  0 or -1: 0 for success
 *   SIDE EFFECTS: Fills the passed in dentry structure
 */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t* dentry)
{
	int i;
	int flag=0;
	int len= strlen((int8_t*)fname);

	dentry_t * dentries_array = calculate_dentries_start();

	for(i=0; i < fs_stats[0]; i++)
	{
		if(len > strlen((int8_t*)dentries_array[i].filename))
		{
		if((strlen((int8_t*)dentries_array[i].filename) == len) && (0 == strncmp((int8_t*)dentries_array[i].filename, (int8_t*)fname, len)) )
		{
			flag = 1;
			break;
		}
		}
		else
		{
		if(0 == strncmp((int8_t*)dentries_array[i].filename, (int8_t*)fname, strlen((int8_t*)dentries_array[i].filename)))
		{
			flag = 1;
			break;
		}
		}
	}

	if(flag==1)
	{
		*dentry = dentries_array[i];
		return 0;
	}

	return -1;


}

/*
 * read_dentry_by_index
 *   DESCRIPTION: fills the passed in dentry structure with the data for the directory at the given index
 *   INPUTS: index, dentry_t * dentry: dentry index and dentry structure
 *   OUTPUTS: none
 *   RETURN VALUE:  0 or -1: 0 for success
 *   SIDE EFFECTS: Fills the passed in dentry structure
 */

int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry)
{

	if(index <0 || index > fs_stats[0])
		return -1;

   	dentry_t * dentries_array = calculate_dentries_start();
	*dentry = dentries_array[index];
	return 0;

}

//------------------------------------------------------------------------------------------------------
// int32_t , void * , int32_t
int32_t read_dir(int32_t fd,  void * buf, int32_t length)
{
	// Check if offset is beyond file - return 0;
	dentry_t * curr_dentry;
	int32_t file_num = curr_pcb_ptr->fd_A[fd].pos;

	if(read_dentry_by_index(file_num, curr_dentry) == -1)
	{
		return 0;
	}
	curr_pcb_ptr->fd_A[fd].pos++;
	buf = (int8_t *)buf;
	strcpy(buf, (int8_t *)curr_dentry->filename);
	return strlen(buf);
}

int32_t read_file_dir()
{
	return 0;
}

//------------------------------------------------------------------------------------------------------
// The wrapper function that actually reads the data from the particular inode
int32_t read_data_file(int32_t fd,  void* buf, int32_t length)
{
	int32_t length_read = 0;
	length_read = read_data(curr_pcb_ptr->fd_A[fd].inode_num, curr_pcb_ptr->fd_A[fd].pos, buf, length);
	if(length_read <= 0)
	{
		return length_read;
	}
	curr_pcb_ptr->fd_A[fd].pos += length_read;
	// terminal_printf("Read size - %d\n", length_read);
	return length_read;
}

/*
 * read_data
 *   DESCRIPTION: Reads data from a specified file
 *   INPUTS: uint32_t inode_num: inode number, uint32_t offset: offset of bytes,  uint8_t * buf: buffer to store the byte in, uint32_t length: no of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE:  0 or -1: 0 for success
 *   SIDE EFFECTS: Fills the passed in dentry structure
 */

//Add appropriate changes cat cating ELF format
int32_t read_data(uint32_t inode_num, uint32_t offset,  uint8_t * buf, uint32_t length)
{
	int count = 0;
	uint32_t file_size;
	int data_index;
	int data_to_copy;
	uint32_t data_ptr;

	inode_t * inodes_start;
	uint32_t data_start = (bootaddr + fs_stats[1]*BYTES_PER_BLOCK + BYTES_PER_BLOCK);
	//checking to see that inode is valid
	if(inode_num >= fs_stats[1] || inode_num < 0)
	{
		return -1;
	}

	inodes_start = calculate_inode_start();

	file_size = inodes_start[inode_num].val[0];

	if(offset >= file_size)
		return 0;

	if( (length + offset) > file_size)
	{
		length = file_size - offset;
	}

	/*Check for invalid data block*/

	data_index =  (offset/BYTES_PER_BLOCK)+1; 			//plus 1 because data numbers starts from 1 and not 0.

	if(inodes_start[inode_num].val[data_index] > fs_stats[2])
	{
		return -1;
	}

	count = 0;
	data_ptr=(data_start + (inodes_start[inode_num].val[data_index]* BYTES_PER_BLOCK)+(offset%BYTES_PER_BLOCK));

	data_to_copy = length;

	if((length+(offset%BYTES_PER_BLOCK)) >BYTES_PER_BLOCK)
	{
		data_to_copy=(BYTES_PER_BLOCK-(offset%BYTES_PER_BLOCK));
	}

	data_index++;
	while(count < length)
	{
		memcpy(buf, (uint8_t*)data_ptr, data_to_copy);
		count += data_to_copy;
		buf += data_to_copy;


 		if(inodes_start[inode_num].val[data_index]> fs_stats[2])
 		{
 			return -1;
 		}

 		data_ptr = (data_start + (inodes_start[inode_num].val[data_index] * BYTES_PER_BLOCK));
		data_index++;
		data_to_copy = length-count;

		if(data_to_copy > BYTES_PER_BLOCK)
		{
			data_to_copy = BYTES_PER_BLOCK;
		}
	}
	return length;
}

//----------------------------------------------------------------------------------------
// FS Testing Functions

/*
 * print_file_names
 *   DESCRIPTION: Prints the file names
 *   INPUTS:
 *   OUTPUTS: Prints the file names
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: none
 */

void print_file_names()
{
	dentry_t * Mydentry;
	int k=0;
	for(k=0; k < NUM_DENTRY; k++)
	{
		read_dentry_by_index(k, Mydentry);
		terminal_printf("%s\n", Mydentry->filename);
	}
}

/*
 * print_file_size
 *   DESCRIPTION: Prints filenames and sizes
 	 INPUTS: none
 *   OUTPUTS: Prints filenames and sizes
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: none
 */
void print_file_size()
{
	dentry_t  Mydentry;
	int k=0;
	for(k=0; k < NUM_DENTRY; k++)
	{
		if (read_dentry_by_index(k, &Mydentry)==-1) continue;

		if(&Mydentry!=NULL && strlen((int8_t*)Mydentry.filename)==31)
		{
			uint32_t size  = get_file_size(&Mydentry);
			terminal_printf("%s    |    %d\n", Mydentry.filename, size);

		}
	}
}


/*
 * print_file_names
 *   DESCRIPTION: Prints file contents
 	 INPUTS:  char * p,  uint32_t offset, uint32_t length
 *   OUTPUTS: none
 *   RETURN VALUE: Prints file contents
 *   SIDE EFFECTS: none
 */

void print_file(char * p,  uint32_t offset, uint32_t length)
{
	dentry_t  Mydentry;
	int k;
	int flag = 0;

	if(0 == read_dentry_by_name((uint8_t*)p, &Mydentry))
	  {
			uint8_t buf[length];

			 if(length==read_data(Mydentry.inode_num, offset,  buf, length ))
			 {

			 	if(buf[0] == ELF_FORMAT)
			 	{
			 		flag = 1;
			 	}

		 		for(k=0; k < length; k++)
		  		{
		  			if(flag==1){
		  				terminal_printf("%x", buf[k]>>4);
						terminal_printf("%x", buf[k]&(0xF));
					}
		  			else
		  				terminal_printf("%c", buf[k]);
		 		 }
				 terminal_printf("\n");
			 }
			 else
			 {
			 	terminal_printf("Problem in reading data\n");
			 }
	  }
	  else
	  {
	  	terminal_printf("File does not exist\n");
	  }
}

/*
 * get_file_size
 *   DESCRIPTION: returns the file size
 	 INPUTS:  dentry_t * Mydentry
 *   OUTPUTS: none
 *   RETURN VALUE: returns the size of the file whose dentry is passed in
 *   SIDE EFFECTS: none
 */
uint32_t get_file_size(dentry_t * Mydentry)
{
	inode_t * inode_start = calculate_inode_start();

	uint32_t size  = inode_start[Mydentry->inode_num].val[0];
	return size;
}

/*
 * file_open
 *   DESCRIPTION: none
 	 INPUTS: const uint8_t* filename
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */


int32_t file_open(const uint8_t* filename)
{
	return 0;
}

/*
 * file_close
 *   DESCRIPTION: none
 	 INPUTS:  int32_t fd
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

int32_t file_close(int32_t fd)
{
	return 0;
}

/*
 * file_write
 *   DESCRIPTION: returns the file size
 	 INPUTS:  int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
//read only file system
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}
