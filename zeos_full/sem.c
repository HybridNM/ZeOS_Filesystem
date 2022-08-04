#include <sem.h>
#include <errno.h>
#include <sched.h>

#define N_SEMAFORS 20

struct sem {
	unsigned id;
	unsigned count;
	struct list_head blocked;
	int owner; //Process ID that initialized the semaphore 
	int used; // 0: free to use, 1: used, <0: in process to be freed
};

struct sem semafors[N_SEMAFORS];

void initialize_sem(void)
{
	int i;
	for (i=0; i<N_SEMAFORS; i++) {
		semafors[i].used = 0;
		semafors[i].id = 0xDEAD;
		semafors[i].owner = 0xDEAD;
		semafors[i].count = 0xDEAD;
	}
}

struct sem * get_semafor( int n_sem ) 
{
	int i;
	struct sem *s;
	for (i=0; i<N_SEMAFORS; i++)  {
		s = &semafors[i];
		if (s->id == (unsigned) n_sem) return s;
	}
	return 0;
}

struct sem* get_free_semafor(void) {
	int i;
	struct sem *s;
	for (i=0; i<N_SEMAFORS; i++)  {
		s = &semafors[i];
		if (s->used == 0) return s;
	}
	return 0;
}

int sem_init( int n_sem, unsigned int value)
{
	struct sem* s;
	if ((s = get_semafor(n_sem)) != 0) return -EINVAL;
	if ((s = get_free_semafor()) == 0) return -2;
	if (s->used != 0) return -EINVAL;
	s->used = 1;
	s->count = value;
	s->owner = current()->PID;
	s->id = n_sem;
	INIT_LIST_HEAD(&s->blocked);
	return 0;
}

int sem_wait( int n_sem )
{
	struct sem* s;
	int res = -1;
	if ((s = get_semafor(n_sem)) == 0) return -EINVAL;
	if (s->used <= 0) return -EINVAL;
	if (s->count > 0) { 
		s->count--;
	} else {//Block
		res = 0; 
		update_process_state_rr(current(), &s->blocked);
		sched_next_rr();
		//Abuse used value...
		if (s->used < 0 ) { //The semaphore has been destroyed
			s->used ++;
			return -2;
		}
	}
	return res;
}

int sem_signal( int n_sem )
{
	struct sem* s;
	if ((s = get_semafor(n_sem)) == 0) return -EINVAL;
	if (s->used <= 0) return -EINVAL;
	if (list_empty(&s->blocked)) { 
		s->count++;
	} else {//UnBlock
		struct list_head *p;
		p = list_first (&s->blocked);
		list_del (p);

		//Schedule process to be executed again
		list_add_tail (p, &readyqueue);
		// update_process_state_rr (p, &readyqueue); ???
	}
	return 0;
}

int internal_sem_destroy(struct sem *s)
{
	struct list_head *p;
	struct list_head *tmp;
	if (s->used <= 0) return -EINVAL;
	if (s->owner != current()->PID) return -EINVAL;
	s->used = 0;
	s->id = 0xDEAD;
	list_for_each_safe (p, tmp, &s->blocked) {
		list_del (p);
		s->used --; //Abuse the used value
		//Schedule process to be executed again
		list_add_tail (p, &readyqueue);
	}
	return 0;
}
int sem_destroy( int n_sem )
{
	struct sem* s;
	if ((s = get_semafor(n_sem)) == 0) return -EINVAL;
	return internal_sem_destroy(s);
}

void destroy_owned_sems(int pid)
{
	int i;
	for (i=0; i<N_SEMAFORS; i++) {
		if (semafors[i].used > 0) {
			if (semafors[i].owner == pid) {
				internal_sem_destroy(&semafors[i]);
			}
		}
	}
}
