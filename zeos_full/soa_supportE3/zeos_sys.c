#include <sched.h>
#include <mm.h>
#include <mm_address.h>
#include <io.h>
#include <devices.h>
#include <utils.h>

#include <schedperf.h>

#define EINVAL 22
#define EBADF 9


int sys_set_sched_policy(int policy) {

	if ((policy != 0) && (policy !=1))
		return -EINVAL;
	if (policy == 0) {
		sched_next=sched_next_rr;
		update_process_state=update_process_state_rr;
		needs_sched=needs_sched_rr;
		update_sched_data=update_sched_data_rr;
	} else {
		sched_next=sched_next_fcfs;
		update_process_state=update_process_state_fcfs;
		needs_sched=needs_sched_fcfs;
		update_sched_data=update_sched_data_fcfs;

	}
	return 0;
}

sys_read(int fd, char *buffer, int size) {
	if (fd != 0) return -EBADF;
	if (size < 0) return -EINVAL;
	return sys_read_console(buffer,size);
}

