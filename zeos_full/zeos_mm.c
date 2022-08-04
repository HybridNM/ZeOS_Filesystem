#include <sched.h>
#include <mm.h>


void monoprocess_init_addr_space() {
	set_user_pages(&task[0].task);
	set_cr3(get_DIR(&task[0].task));
}
