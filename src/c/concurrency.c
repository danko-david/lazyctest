
#include "concurrency.h"

int short_lock_init(short_lock* spin)
{
	return pthread_spin_init(spin, 0);
}

int short_lock_lock(short_lock* spin)
{
	return pthread_spin_lock(spin);
}

int short_lock_trylock(short_lock* spin)
{
	return pthread_spin_trylock(spin);
}

int short_lock_unlock(short_lock* spin)
{
	return pthread_spin_unlock(spin);
}

int short_lock_destroy(short_lock* spin)
{
	return pthread_spin_destroy(spin);
}



int long_lock_init(long_lock* mutex)
{
	return pthread_mutex_init(mutex, NULL);
}

int long_lock_lock(long_lock* mutex)
{
	return pthread_mutex_lock(mutex);
}

int long_lock_trylock(long_lock* mutex)
{
	return pthread_mutex_trylock(mutex);
}

int long_lock_unlock(long_lock* mutex)
{
	return pthread_mutex_unlock(mutex);
}

int long_lock_destroy(long_lock* mutex)
{
	return pthread_mutex_destroy(mutex);
}


int thread_setup_after_start()
{
	pthread_detach(pthread_self());
	return 0;
}

int thread_init_env(void)
{
	return 0;
}

int thread_destroy_env(void)
{
	return 0;
}

int thread_cond_wait_init(struct conditional_wait* cw)
{
	pthread_mutex_init(&cw->mutex, NULL);
	pthread_cond_init(&cw->condition, NULL);
	return 0;
}

int thread_cond_wait_destroy(struct conditional_wait* cw)
{
	pthread_mutex_destroy(&cw->mutex);
	pthread_cond_destroy(&cw->condition);
	return 0;
}

void thread_cond_wait_notify(struct conditional_wait* cw)
{
	pthread_mutex_lock(&cw->mutex);
	pthread_cond_broadcast(&cw->condition);
	pthread_mutex_unlock(&cw->mutex);
}

void thread_cond_wait_wait(struct conditional_wait* cw)
{
	pthread_cond_wait(&cw->condition, &cw->mutex);
}

int thread_cond_wait_lock(struct conditional_wait* cw)
{
	return pthread_mutex_lock(&cw->mutex);
}

int thread_cond_wait_unlock(struct conditional_wait* cw)
{
	return pthread_mutex_unlock(&cw->mutex);
}

int thread_cond_wait_trylock(struct conditional_wait* cw)
{
	return pthread_mutex_trylock(&cw->mutex);
}

