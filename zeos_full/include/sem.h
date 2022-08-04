#ifndef __SEM_H__
#define __SEM_H__

#include <list.h>

int sem_init( int n_sem, unsigned int value);
int sem_wait( int n_sem );
int sem_signal( int n_sem );
int sem_destroy( int n_sem );
void destroy_owned_sems(int pid);
#endif /* __SEM_H__ */
