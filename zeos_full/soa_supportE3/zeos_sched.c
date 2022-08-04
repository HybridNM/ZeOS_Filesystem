#include <schedperf.h>
#include <mm.h>
#include <utils.h>
#include <stats.h>


void (*sched_next)(void);
void (*update_process_state) (struct task_struct *, struct list_head *) ;
int (*needs_sched)();
void (*update_sched_data)();

void init_sched_policy(){
	sched_next=sched_next_fcfs;
	update_process_state=update_process_state_fcfs;
	needs_sched=needs_sched_fcfs;
	update_sched_data=update_sched_data_fcfs;
}


void sched_next_fcfs() {
	struct list_head *l;
	struct task_struct *t;
	struct stats *st;
        if (!list_empty(&readyqueue)) {
		l= list_first(&readyqueue);
		list_del(l);
		t=list_head_to_task_struct(l);
	} else {
		t =idle_task;
	}
	st=get_task_stats(t);
	st->ready_ticks += (get_ticks() - st->elapsed_total_ticks);
	st->elapsed_total_ticks = get_ticks();
	
	task_switch((union task_union *)t);
}

void update_process_state_fcfs(struct task_struct *t, struct list_head *dest_queue) {
	struct list_head *src;
	src = get_task_list (t);
	if (src->next != 0)
		list_del (src);
	if (dest_queue != NULL)
		if (t != idle_task) 
			list_add_tail(src, dest_queue);
}

int needs_sched_fcfs(){
	if ((current() == idle_task) && !list_empty(&readyqueue))
		return 1;
	return 0;
}

void update_sched_data_fcfs() {
}



