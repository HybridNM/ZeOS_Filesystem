/*
 * sys.c - Syscalls implementation
 */

#include <devices.h>
#include <utils.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <errno.h>
#include <interrupt.h>
#include <sem.h>
#include <types.h>
#include <fat32.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern unsigned PID;

open_file_table_entry open_file_table[OPEN_FILE_TABLE_SIZE];


void init_open_file_table()
{
	for (int i=0; i<OPEN_FILE_TABLE_SIZE; i++)
	{
		open_file_table[i].startCluster = 0;
		open_file_table[i].openCount = 0;
		open_file_table[i].access_offset = 0;
		open_file_table[i].permissions = -1;
	}
}

int check_fd(int fd, int permissions)
{
	// Get a reference to the process' channel table and the open file table entry
	channel_table_entry *process_ch_t = get_CHT( current() );
	int OFT_entry = process_ch_t[fd-2].OFT_entry_num;

	// fd=1 is the screen, and can't be read
	if (fd == 1 && permissions == LECTURA) return -EBADF;
	
	// startCluster=0 means the entry is not in use, therefore, return bad file number error
	if (fd != 1 && open_file_table[OFT_entry].startCluster == 0) return -EBADF;

	// Check that operation permissions match with the open file's
	if (permissions != open_file_table[OFT_entry].permissions) return -EACCES;

	return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_gettime()
{
	return zeos_ticks;
}


#define TAM_BUFFER 512


int sys_write(int fd, char *buffer, int nbytes) 
{
	char localbuffer [TAM_BUFFER];
	int bytes_left;
	int ret;

	// Parameter check
	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	// Write to console
	if (fd == 1)
	{
		bytes_left = nbytes;
		while (bytes_left > TAM_BUFFER) 
		{
			copy_from_user(buffer, localbuffer, TAM_BUFFER);
			ret = sys_write_console(localbuffer, TAM_BUFFER);
			if (ret < 0 )
				return ret;
			bytes_left-=ret;
			buffer+=ret;
		}
		if (bytes_left > 0) 
		{
			copy_from_user(buffer, localbuffer,bytes_left);
			ret = sys_write_console(localbuffer, bytes_left);
			if (ret < 0 )
				return ret;
			bytes_left-=ret;
		}
		return (nbytes-bytes_left);
	}
	// Write to file
	if (fd > 1)
	{
		// Get a reference to the process' channel table and the open file table entry
		channel_table_entry *process_ch_t = get_CHT( current() );
		int OFT_entry = process_ch_t[fd-2].OFT_entry_num;

		// Call the file system function
		DWord firstCluster = open_file_table[OFT_entry].startCluster;
		int offset = open_file_table[OFT_entry].access_offset;
		int ret = writeFile(firstCluster, buffer, nbytes, offset);

		// If the write doesn't return 0 we can assume there was an I/O error
		if (ret != 0) return -EIO;
		else
		{
			open_file_table[OFT_entry].access_offset += nbytes;
			return ret;
		}
	}
	// Should never get here
	return -EBADF;
}

int sys_read(int fd, char *buffer, int nbytes)
{
	int ret, bytes_left;

	// Parameter check
	if ((ret = check_fd(fd, LECTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	// Skip verify step, since reading is not as dangerous as writing


	// Get a reference to the process' channel table and the open file table entry
	channel_table_entry *process_ch_t = get_CHT( current() );
	int OFT_entry = process_ch_t[fd-2].OFT_entry_num;

	// Call the file system function
	DWord firstCluster = open_file_table[OFT_entry].startCluster;
	int offset = open_file_table[OFT_entry].access_offset;
	ret = readFile(firstCluster, buffer, nbytes, offset);

	if (ret != 0) return -EIO; // TO DO: copy to user if free time
	// else
	open_file_table[OFT_entry].access_offset += nbytes;
	return ret;
} 


int sys_open(char *path, int mode)
{
	// TO DO: ver que hacer si hay una entrada al mismo fichero con otro modo

	DWord fileCluster;
	int numReferences = 1; // Number of refs for the open file
	int i;
	int OFT_entry;

	channel_table_entry *process_ch_t = get_CHT( current() );
	
	// Get the file cluster number, if the return value is 0 then 
	//	the file doesn't exist and should be created
	fileCluster = searchFile(path);
	if (fileCluster == 0)
	{
		// TO DO: Parse and separate filename
		int ret = createFile(path, "file1     ", 0);
	}

	// Look for an entry in the channel table
	for (i=0; i<CHANNEL_TABLE_SIZE; i++)
	{
		// Found a free entry
		if (process_ch_t[i].fd != i+2)
		{
			// Now look for an available entry in the OFT
			for (i=0; i<CHANNEL_TABLE_SIZE; i++)
			{
				if (open_file_table[i].startCluster == 0) 
				{
					OFT_entry = i;
					break;
				}
			}

			// Channel table assigns the fd number based on its possition in the table,
			//  so that fd_number = ch_table_entry_num+2, to skip fd=0 and fd=1, which are reserved
			process_ch_t[i].fd = i+2;
			process_ch_t[i].OFT_entry_num = OFT_entry;

			// Go through the open file table to look for other entries pointing to the same file
			// In that case we have to increase the number of references for those entries too
			for (int j=0; j<OPEN_FILE_TABLE_SIZE; j++)
			{
				if (open_file_table[j].startCluster == fileCluster)
				{
					// We could also just take the openCount of a different entry and add 1
					open_file_table[j].openCount++;
					numReferences++;
				}
			}

			// Now that everything else is taken care of the OFT entry for this process can
			//  be created, and filled with the corresponding information
			open_file_table[OFT_entry].startCluster = fileCluster;
			open_file_table[OFT_entry].openCount = numReferences;
			open_file_table[OFT_entry].access_offset = 0;
			open_file_table[OFT_entry].permissions = mode;
			
			// Return the fd
			return i+2;
		}
	}
	// No available space for new open files
	return -EMFILE;
}


int sys_close(int fd)
{
	// Get a reference to the process' channel table
	channel_table_entry *process_ch_t = get_CHT( current() );

	// If the entry number 'fd' is not equal to fd (will usually be 0 in that case)
	//  then the file is not open, and should return a 'bad file' error
	if (process_ch_t[fd].fd != fd+2) return -EBADF;
	
	// Get the Open File Table entry 
	int OFT_entry = process_ch_t[fd-2].OFT_entry_num;

	// Get the first cluster of the file to search for other possible entries to the same file
	DWord fileCluster = open_file_table[OFT_entry].startCluster;

	// Reduce the number of references of all entries to the same file if there are multiple
	if (open_file_table[OFT_entry].openCount > 1)
	{
		for (int i=0; i<OPEN_FILE_TABLE_SIZE; i++)
		{
			open_file_table[i].startCluster == fileCluster;
			open_file_table[i].openCount--;
		}
	}
	// Clear the OFT entry for this process
	open_file_table[OFT_entry].startCluster = 0;
	open_file_table[OFT_entry].openCount = 0;
	open_file_table[OFT_entry].access_offset = 0;
	open_file_table[OFT_entry].permissions = -1;

	return 0;
}


int sys_unlink(char * path) // A fancy name for the delete function
{
	// First check if the file is open by a different process
	DWord fileCluster = searchFile(path);
	
	for (int i=0; i<OPEN_FILE_TABLE_SIZE; i++)
	{
		// If the file is open the function returns with an error
		if (open_file_table[i].startCluster == fileCluster)
		{
			return -EACCES; // Not the ideal code but we can make do with it
		}
	}

	return deleteFile(path, "file1     "); // TO DO: cut path into 2 parts
}


int sys_lseek(int fd, int offset, int whence) // whence=0: SEEK_SET, whence=1: SEEK_CUR
{
	// Invalid whence value
	if (whence != 0 && whence != 1) return -EINVAL;
	
	// Get a reference to the process' channel table
	channel_table_entry *process_ch_t = get_CHT( current() );

	// File not open by the process
	if (process_ch_t[fd-2].fd != fd) return -EBADF;

	// Get the Open File Table entry 
	int OFT_entry = process_ch_t[fd-2].OFT_entry_num;
	int ret_seek;

	if (whence == 0) // SEEK_SET
	{
		ret_seek = offset;
		open_file_table[OFT_entry].access_offset = offset;
	}
	else // SEEK_CUR
	{
		ret_seek = open_file_table[OFT_entry].access_offset + offset;
		open_file_table[OFT_entry].access_offset = ret_seek;
	}
	// Return the final offset
	return ret_seek;
}



int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork()
{
	return 0;
}

int sys_fork()
{
	struct list_head *l;
	struct task_struct *child;
	int i;
	int j;
	int v[NUM_PAG_DATA];
#include<errno.h>	
	// Get a Free PCB
	if (list_empty( &freequeue )) return -ENOMEM; 
	l = list_first( &freequeue );
	child = list_head_to_task_struct( l );

	// Find enough free frames
	for (i = 0; i < NUM_PAG_DATA; i++ ) {
		v[i] = alloc_frame();
		if (v[i] < 0) {
			// Free previously allocated frames
			for (j=0; j < i ; j++ ) {
				free_frame( v[j] );
			}
			return -ENOMEM;
		}
	}
	list_del( l );

	// Inherit System DATA
	copy_data( current(), child, sizeof(union task_union) );
	
	// Inherit Address Space
	allocate_DIR( child );

	page_table_entry *ptChild = get_PT( child );
	page_table_entry *ptFather = get_PT( current() );
	// Kernel is shared
	for (i = 0; i < NUM_PAG_KERNEL; i ++) {
		ptChild[i].entry = ptFather[i].entry;
	}
	// User Code is shared
	for (i = NUM_PAG_KERNEL; i < NUM_PAG_KERNEL + NUM_PAG_CODE; i ++) {
		ptChild[i].entry = ptFather[i].entry;
	}
	// Inherit User Data
	for (i = NUM_PAG_KERNEL+NUM_PAG_CODE; i < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; i ++) {
		set_ss_pag( ptChild, i, v[i-NUM_PAG_KERNEL-NUM_PAG_CODE]);
		// Create temporal entries in the current pagetable
		set_ss_pag( ptFather, i+NUM_PAG_DATA, v[i-NUM_PAG_KERNEL-NUM_PAG_CODE]);
		// Copy each user data page to the temporal entry 
		copy_data( (void*) (i << 12), (void*) ((i + NUM_PAG_DATA) << 12), PAGE_SIZE );
		// Delete temporal entry
		del_ss_pag( ptFather, i+NUM_PAG_DATA );
	}
	set_cr3( get_DIR(current()) ); // Remove access to temporal entries

	// Assign New PID
	child->PID = ++PID;

	// Initialize remaining fields
	init_stats(&child->stats);

	// Prepare child stack
	union task_union * cu = (union task_union*) child;
	// pos: Position in the stack above the saved registers in the handler
	// 	5: # of registers saved by the hw
	// 	11: # of registers saved by the SAVE_ALL macro
	// 	1: saved address to return back to the system_call_handler
	int pos = KERNEL_STACK_SIZE - 5 - 11 - 1;
	cu->stack[pos - 1] = (unsigned long) ret_from_fork;
	cu->stack[pos - 2] = 0;
	child-> kernel_esp = (char *) &cu->stack[pos-2];

	// Queue process into READY
	list_add_tail( &child->list, &readyqueue );
	// Return the new PID
	return PID;
}

int *find_dir_usage(struct task_struct *t) 
{
	int i;
	for(i=0; i<NR_TASKS; i++) {
		if (t->dir_pages_baseAddr == (page_table_entry*)&dir_pages[i]) return &dir_usage[i];
	}
	return 0;
}
void sys_exit()
{  
struct task_struct *t = current();

	int *dir = find_dir_usage(t);
	*dir = *dir - 1;
	if (*dir==0) {
		free_user_pages(t);
	}
	destroy_owned_sems(t->PID);
	t->PID = -1; //Mark process as dead
	update_process_state_rr(current(), &freequeue);
	sched_next_rr();
}
int sys_getstats(int pid, struct stats *s)
{
	int res = -1;
	int i;
	if (pid <0) return -1;
	if (s == NULL) return -EFAULT;
	if (!access_ok(VERIFY_WRITE, s, sizeof(struct stats))) return -EFAULT;
	for (i = 0; i < NR_TASKS; i++) {
		struct task_struct *t = &task[i].task;
		if (t->PID == pid) {
			unsigned long tmp;	//Temporal value to avoid overwriting remaining_ticks
			finishSystemTime(t);
			tmp = t->stats.remaining_ticks; //Beginning time for process
			t->stats.elapsed_total_ticks -= tmp;	//Total ticks since the creation of the process
			t->stats.remaining_ticks = quantum_left;
			copy_to_user(&t->stats, s, sizeof(struct stats));
			t->stats.remaining_ticks = tmp;	//Restore value
			return 0; //FOUND, exit
		}
	}
	return res;
}


int sys_clone(void (*function)(void), void *stack)
{
	int res = -1;

	if (function == NULL) return -EFAULT;
	if (stack == NULL) return -EFAULT;
	if (!access_ok(VERIFY_READ, function, sizeof(void *))) return -EFAULT;
	if (!access_ok(VERIFY_WRITE, stack, sizeof(void *))) return -EFAULT;

	struct list_head *l;
	struct task_struct *child;
#include<errno.h>	
	// Get a Free PCB
	if (list_empty( &freequeue )) return -ENOMEM; 
	l = list_first( &freequeue );
	list_del(l);
	child = list_head_to_task_struct( l );

	// Inherit System DATA
	copy_data( current(), child, sizeof(union task_union) );
	
	// Increase usage of Directory
	int *dir = find_dir_usage(child);
	//(*dir)++;
	*dir = *dir + 1;

	// Assign New PID
	child->PID = ++PID;

	// Initialize remaining fields
	init_stats(&child->stats);

	// Prepare child stack
	union task_union * cu = (union task_union*) child;
	// pos: Position in the stack above the saved registers in the handler
	// 	5: # of registers saved by the hw
	// 	11: # of registers saved by the SAVE_ALL macro
	// 	1: saved address to return back to the system_call_handler
	int pos = KERNEL_STACK_SIZE - 5 - 11 - 1;
	cu->stack[pos - 1] = (unsigned long) ret_from_fork;
	cu->stack[pos - 2] = 0;
	child-> kernel_esp = (char *) &cu->stack[pos-2];
	cu->stack[KERNEL_STACK_SIZE-2] = (unsigned long)stack;
	cu->stack[KERNEL_STACK_SIZE-5] = (unsigned long)function;

	// Queue process into READY
	list_add_tail( &child->list, &readyqueue );
	// Return the new PID
	res = child->PID;

	return res;
}

int sys_sem_init( int n_sem, unsigned int value)
{
	return sem_init(n_sem, value);
}

int sys_sem_wait( int n_sem )
{
	return sem_wait(n_sem);
}

int sys_sem_signal( int n_sem )
{
	return sem_signal(n_sem);
}

int sys_sem_destroy( int n_sem )
{
	return sem_destroy(n_sem);
}

