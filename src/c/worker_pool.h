/*
 * worker_pool.h
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#ifndef WORKER_POOL_H_
#define WORKER_POOL_H_

#include "include.h"
#include "rerunnable_thread.h"

enum worker_pool_status
{
	wp_available,
	wp_shutting_down,
	wp_exited,
};

struct worker_pool
{
	long_lock pool_lock;
	enum worker_pool_status status;

	struct queue_element* free_head;
	struct queue_element* free_tail;

	struct queue_element* busy_head;
	struct queue_element* busy_tail;
};


/**
 * Contains all necessary elements to thread (inside this structure)
 * can be used in the pool:
 * - queue_element: to can be stored in the pool's busy/free list
 * - the owner queue reference (to can push himself to free list when done.)
 * - the thread and current task related stuffs.
 * */
struct pool_thread
{
	struct queue_element elem;
	struct rerunnable_thread thread;
	struct worker_pool* pool;
	void (*executor)(void*);
	void* param;
};

int wp_init(struct worker_pool* pool);

int wp_submit_task(struct worker_pool* wp, void (*func)(void*), void* param);

enum worker_pool_status wp_get_status(struct worker_pool* wp);

int wp_shutdown(struct worker_pool* pool);

int wp_wait_exit(struct worker_pool* pool);

int wp_destroy(struct worker_pool* pool);

/*
void lxc_init_thread_pool();

void lxc_wait_thread_pool_shutdown();

void lxc_submit_asyncron_task(void (*funct)(void*), void* param);

struct pool_thread
{
	struct queue_element queue_element;
	struct rerunnable_thread rerunnable;
};
*/

#endif /* WORKER_POOL_H_ */
