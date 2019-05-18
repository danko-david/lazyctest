
#ifndef CONCURRENCY_H_
#define CONCURRENCY_H_

#include <stddef.h>
#include <pthread.h>

#define short_lock pthread_spinlock_t
#define long_lock pthread_mutex_t
typedef pthread_t* thread_handle;

struct conditional_wait
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
};

int short_lock_init(short_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_lock(short_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_trylock(short_lock*);
int short_lock_unlock(short_lock*);
int short_lock_destroy(short_lock*);


int long_lock_init(long_lock*);
int long_lock_lock(long_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int long_lock_trylock(long_lock*);
int long_lock_unlock(long_lock*);
int long_lock_destroy(long_lock*);

#endif
