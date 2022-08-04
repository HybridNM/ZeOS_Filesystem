#include <schedperf.h>
#include <list.h>

typedef struct iorb{
	struct task_struct *t;
	int io_ticks;
	struct list_head  l;
} iorb_t;

extern iorb_t pending_io[NR_TASKS];

void zeos_console_init();
int zeos_read_console_emul();
void zeos_update_read_console_emul();
