/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

#define CHANNEL_TABLE_SIZE 10

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;					/* Process ID. This MUST be the first field of the struct. */
  unsigned quantum;			/* Initial QUANTUM per process. */
  struct list_head list;	/* List anchor */
  char *kernel_esp;			/* Task-Switch Restore point */
  page_table_entry * dir_pages_baseAddr;
  struct stats stats;
  channel_table_entry channel_table[CHANNEL_TABLE_SIZE];
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;
extern int dir_usage[NR_TASKS];

extern struct list_head readyqueue;
extern struct list_head freequeue;

extern int quantum_left;



#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t);

page_table_entry * get_DIR (struct task_struct *t);

channel_table_entry * get_CHT (struct task_struct *t);

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void scheduling();
void finishSystemTime(struct task_struct* t);

#endif  /* __SCHED_H__ */
