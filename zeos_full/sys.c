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

#define LECTURA 0
#define ESCRIPTURA 1

extern unsigned PID;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES;
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

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		if (ret < 0 )
			return ret;
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		if (ret < 0 )
			return ret;
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
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

