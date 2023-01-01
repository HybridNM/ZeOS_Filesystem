/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}

extern struct list_head blocked;

struct list_head freequeue;
struct list_head readyqueue;

int quantum_left;

struct task_struct * idle_task;

int dir_usage[NR_TASKS];

unsigned PID = 0;

#define DEFAULT_QUANTUM 30

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

/* get_CHT - Returns the Channel Table address for task 't' */
page_table_entry * get_CHT (struct task_struct *t) 
{
	return t->channel_table;
}


int allocate_DIR(struct task_struct *t) 
{
	int i = 0;
	for (i = 0; i < NR_TASKS; i ++ ) {
		if (dir_usage[i] == 0) break;
	}
	if (i == NR_TASKS) return -1;
	dir_usage[i]++; //Increase directory usage
	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[i]; 

	return i;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	// Get a free PCB
	struct list_head *l = list_first( &freequeue );
	list_del(l);
	struct task_struct *idle = list_head_to_task_struct(l);
	union task_union *uidle = (union task_union *) idle;
	
	// Assign pid
	idle -> PID = PID++;

	// Allocate Directory
	allocate_DIR( idle );

	// Prepare context
	uidle->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle;
	uidle->stack[KERNEL_STACK_SIZE - 2] = 0;
	idle->kernel_esp = (char*) &uidle->stack[KERNEL_STACK_SIZE - 2];

	// Initialize idle_task
	idle_task = idle;
	init_stats(&idle->stats);
	idle->quantum = DEFAULT_QUANTUM;
}

void init_task1(void)
{
	// Get a free PCB
	struct list_head *l = list_first( &freequeue );
	list_del(l);
	struct task_struct *init = list_head_to_task_struct(l);
	union task_union *uinit = (union task_union *)init;

	// Assign PID
	init->PID = PID++;

	// Allocate Directory
	allocate_DIR( init );

	// Complete Address Space
	set_user_pages( init );

	// Enable new Address Space
	set_cr3( get_DIR( init ) );

	// Update TSS
	tss.esp0 = (DWord) &uinit->stack[KERNEL_STACK_SIZE];

	//set sched info
	init->quantum = DEFAULT_QUANTUM;
	quantum_left = init->quantum;
	init_stats(&init->stats);
}


void set_quantum(struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

int get_quantum(struct task_struct *t) {
	return t->quantum;
}

void init_sched()
{
	int i;

	INIT_LIST_HEAD( &freequeue );
	INIT_LIST_HEAD( &readyqueue );

	// Add all tasks to the freequeue
	for (i = 0; i < NR_TASKS; i ++ ) { 
		task[i].task.PID = -1;
		list_add( &task[i].task.list, &freequeue );
		dir_usage[i] = 0;
	}
	initialize_sem();
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void finishUserTime(struct task_struct* t)
{
	unsigned long temps = get_ticks();
	t->stats.user_ticks += temps - t-> stats.elapsed_total_ticks;
	t->stats.elapsed_total_ticks = temps;
}
void finishSystemTime(struct task_struct* t)
{
	unsigned long temps = get_ticks();
	t->stats.system_ticks += temps - t-> stats.elapsed_total_ticks;
	t->stats.elapsed_total_ticks = temps;
}
void finishReadyTime(struct task_struct* t)
{
	unsigned long temps = get_ticks();
	t->stats.ready_ticks += temps - t-> stats.elapsed_total_ticks;
	t->stats.elapsed_total_ticks = temps;
	t->stats.total_trans ++;
}
void user2system()
{
	finishUserTime(current());
}
void system2user()
{
	finishSystemTime(current());
}
void system2ready()
{
	finishSystemTime(current());
}
void ready2system()
{
	finishReadyTime(current());
}

void init_stats(struct stats *s)
{
	s->user_ticks = 0;
	s->system_ticks = 0;
	s->blocked_ticks = 0;
	s->ready_ticks = 0;
	s->elapsed_total_ticks = get_ticks();
	s->total_trans = 0;
	s->remaining_ticks = get_ticks(); //CREATION TIME!!
}

void inner_task_switch( union task_union *new )
{
	// Update TSS
	tss.esp0 = (DWord) &new->stack[KERNEL_STACK_SIZE];

	if (get_DIR( current() ) != get_DIR( (struct task_struct*)new ) ) {
		// Change user address space
		set_cr3( get_DIR( &new->task ) );
	}

	// Store current EBP value
	char * ebp;
	__asm__ __volatile__ (
			"movl %%ebp, %0"
			: "=g"(ebp)
			);
	current()->kernel_esp = ebp;	

	// Change current's Stack to new's stack
	// Restore ebp
	// Return	
	__asm__ __volatile__ (
			"movl %0, %%esp; \n"
			"popl %%ebp; \n"
			"ret"
			: : "g"(new->task.kernel_esp)
			);
}

void task_switch( union task_union *new )
{
	__asm__ __volatile__ (
		"pushl %%ebx; \n"
		"pushl %%esi; \n"
		"pushl %%edi; \n"
		"pushl 8(%%ebp); \n"
		"call inner_task_switch; \n"
		"add $4, %%esp; \n"
		"popl %%edi; \n"
		"popl %%esi; \n"
		"popl %%ebx; \n"
		::"m"(new):"memory");
}

void scheduling() {
	update_sched_data_rr();
	if (needs_sched_rr()){
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}

}

void update_sched_data_rr() {
	quantum_left--;
}

int needs_sched_rr(){
struct task_struct *current_task = current();

	if ((!quantum_left)||(current_task == idle_task)) {
		if ((current_task == idle_task) && (list_empty(&readyqueue))) {
			quantum_left = current_task->quantum;
			return 0;
		}
		return 1;
	}
	return 0;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst){
	if (t->list.next != 0)
		list_del (&t->list);
	if (dst != NULL)
		if (t != idle_task)
			list_add_tail(&t->list, dst);
}

void sched_next_rr() {
	struct task_struct *t;
	if (!list_empty(&readyqueue)) {
		t = list_head_to_task_struct(list_first(&readyqueue));
		list_del (&t->list);
	} else t = idle_task;

	quantum_left = get_quantum(t);

	finishSystemTime(current());
	if (t != idle_task) finishReadyTime(t);
	task_switch((union task_union *) t);
}

