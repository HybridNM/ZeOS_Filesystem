#include <list.h>
#include <sched.h>
#include <io.h>

#define Z_MAX_SEMS	10

#define EINVAL 1

typedef struct z_sem_struct {
	int used; // 0: free to use, 1: used, <0: in process to be freed
	int counter; // 0-MAX_PROCESSES
	int owner; // process id that creates the semaphores 
	struct list_head sem_blocked;
} z_sem_t;

z_sem_t zeos_sems[Z_MAX_SEMS];

extern struct list_head readyqueue;

extern void zeos_call_handler (void);

/* zeos_sys_init_sems - Initializes semaphores structures */
int zeos_sys_init_sems()
{
	int i;
	for (i = 0; i<Z_MAX_SEMS; i++) {
		z_sem_t *s = &zeos_sems[i];
		s->used = 0;
		INIT_LIST_HEAD (&s->sem_blocked);
	}

	return 0;
}

/* zeos_sys_sem_init - initializes semaphore 'sem_id' with value 'value' */
int zeos_sys_sem_init (int sem_id, int value)
{
	z_sem_t *s;
	if ((sem_id < 0) || (sem_id > Z_MAX_SEMS)) { return -EINVAL;}
	s = &zeos_sems[sem_id];
	if (s->used > 0){
		return -EINVAL;
	}
	if (s->used < 0) {
		printk("\n sem_init: Waiting finalization of blocked processes [");
		while (s->used < 0){	//Blocked process still alive...
			printk(".");
			update_process_state_rr(current(), &readyqueue); //manual schedule to execute childs
			sched_next_rr();
		}
		printk(" Done]\n");
	}
	s->used = 1;
	s->counter = value;
	s->owner = current()->PID;
	INIT_LIST_HEAD (&s->sem_blocked);
	return 0;
}

/* zeos_sys_sem_destroy - destroys semaphore 'sem_id' unblocking all blocked processes */
int zeos_sys_sem_destroy (int sem_id)
{
	struct list_head *p;
	struct list_head *tmp;
	z_sem_t *s;
	if ((sem_id < 0) || (sem_id > Z_MAX_SEMS)) { return -EINVAL;}
	s = &zeos_sems[sem_id];
	if (s->used <= 0){
		return -EINVAL;
	}
	if (s->owner != current()->PID) {
		return -EINVAL;
	}
	s->used = 0;
	list_for_each_safe (p, tmp, &s->sem_blocked) {
		s->used --; //Abuse the used value
		//Schedule process to be executed again
		update_process_state_rr(list_head_to_task_struct(p), &readyqueue);
	}
	return 0;
}

/* zeos_sys_sem_signal - Signals the semaphore 'sem_id', if there is any blocked process, it will be unblocked, otherwise, increase counter */
int zeos_sys_sem_signal (int sem_id)
{
	struct list_head *p;
	z_sem_t *s;
	if ((sem_id < 0) || (sem_id > Z_MAX_SEMS)) { return -EINVAL;}
	s = &zeos_sems[sem_id];
	if (s->used <= 0){
		return -EINVAL;
	}
	if (list_empty (&s->sem_blocked)) {
		s->counter ++;
	} else {
		p = list_first (&s->sem_blocked);

		//Schedule process to be executed again
		update_process_state_rr (list_head_to_task_struct(p), &readyqueue);
	}
	return 0;
}

/* zeos_sys_sem_wait - wait on a semaphore, if the counter is >0 decreases it, otherwise blocks the process */
int zeos_sys_sem_wait (int sem_id)
{
	z_sem_t *s;
	if ((sem_id < 0) || (sem_id > Z_MAX_SEMS)) { return -EINVAL;}
	s = &zeos_sems[sem_id];
	if (s->used <= 0){
		return -EINVAL;
	}
	if (s->counter > 0) {
		s->counter --;
	} else {
		//Block current process and schedule next one
		update_process_state_rr (current(), &s->sem_blocked);
		sched_next_rr ();
		//Abuse used value...
		if (s->used < 0 ) { //The semaphore has been destroyed
			s->used ++;
			return -2;
		}
	}
	return 0;
}

/**
 * Search for pid in the list of blocked tasks
 * @param pid Process ID to find.
 * @return 1 (true) if the process is blocked or 0 (false) if it is not
 */
int zeos_is_blocked_pid(int pid)
{
	int i;
	z_sem_t *s;
	struct list_head *tmp;
	struct task_struct *p;

	for (i = 0; (i < Z_MAX_SEMS) ; i++) {
		s = &zeos_sems[i];
		list_for_each(tmp, &s->sem_blocked) {
			p = list_head_to_task_struct(tmp);
			if (p->PID == pid) {
				return 1;
			}
		}
	}
	return 0;
}

extern int get_quantum(struct task_struct *);
extern void set_quantum(struct task_struct *, int);
/* zeos_sys_nice - Set a new quantum and return the old one */
int zeos_sys_nice (int quantum)
{
	int old = get_quantum(current());
	set_quantum(current(), quantum);
	return old;
}

extern void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

/* to be called from the ZeOS initialization code */
void zeos_init_auxjp()
{
    printk("\n(II) Initializing semaphores..");
	zeos_sys_init_sems();
    printk("OK\n");
    printk("(II) Initializing Backdoor..");
	setTrapHandler(0x82, zeos_call_handler, 3);
    printk("OK\n");
}

