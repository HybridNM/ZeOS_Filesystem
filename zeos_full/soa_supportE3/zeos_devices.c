#include <zeos_devices.h>
#include <sched.h>

#define EAGAIN 11 

iorb_t iorbs[NR_TASKS];

struct list_head pending_reads;
struct list_head console_blocked;
struct list_head free_iorbs;

int sys_read_console(char *buffer, int size){
int ret = size;
        ret = zeos_read_console_emul(size);
        return ret;

}

void zeos_console_init() {
int i;
	INIT_LIST_HEAD(&pending_reads);
	INIT_LIST_HEAD(&console_blocked);
	INIT_LIST_HEAD(&free_iorbs);
	for (i=0;i<NR_TASKS;i++) {
		list_add(&iorbs[i].l, &free_iorbs);
	}
}

int zeos_read_console_emul (int nticks) {
	iorb_t *io;

	if (list_empty(&free_iorbs))
		return -EAGAIN;

	io = list_entry(list_first(&free_iorbs),iorb_t,l);
	list_del(&io->l);
	io->io_ticks = nticks;
	io->t = current();
	list_add_tail(&io->l, &pending_reads);
	//block_process
	block_process(&console_blocked);
	return nticks;
}


void zeos_update_read_console_emul () {
	iorb_t *current_iorb;
	if (!list_empty(&pending_reads)){
		current_iorb = list_entry(list_first(&pending_reads), iorb_t,l);
		current_iorb->io_ticks--;
		if (current_iorb->io_ticks <=0) {
			//emulate io fin: unblock_process
			list_del(&current_iorb->l);
			list_add(&current_iorb->l, &free_iorbs);
			unblock_process(current_iorb->t);
		}
	}

}
