extern int errno;

int read (int fd, char *buf, int size){
int res;
__asm__ __volatile__(
	"pushl %%ebx\n"
	"movl $3, %%eax\n"
	"movl %1, %%ebx\n"
	"movl %2, %%ecx\n"
	"movl %3, %%edx\n"
	"int $0x80\n"
	"movl %%eax, %0\n"
	"popl %%ebx\n"
        : "=g"(res) 
	: "g"(fd), "g"(buf), "g"(size) 
);

if (res < 0) {
	errno = -res;
	return -1;	
}
return res;

}

int set_sched_policy (int policy_id) {
int res;
__asm__ __volatile__(
	"pushl %%ebx\n"
	"movl $17, %%eax\n"
	"movl %1, %%ebx\n"
	"int $0x80\n"
	"movl %%eax, %0\n"
	"popl %%ebx\n"
        : "=g"(res) 
	: "g"(policy_id)
);

if (res < 0) {
	errno = -res;
	return -1;	
}
return res;


}

