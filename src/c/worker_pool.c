/*
 * worker_pool.c
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "worker_pool.h"

static void on_release
(
	struct rerunnable_thread* rrt,
	enum rrt_callback_point point,
	void (*func)(void*),
	void* param
)
{
	if(rrt_after_become_idle == point)
	{
		//UNUSED(rrt);
		//UNUSED(func);
		//printf("on_release pool_thread ptr: %p\n", param);
		struct pool_thread* task  = (struct pool_thread*) param;
		struct worker_pool* pool = task->pool;
		long_lock_lock(&pool->pool_lock);

		queue_pop_intermediate_element
		(
			&pool->busy_head,
			&task->elem,
			&pool->busy_tail
		);

		queue_add_element
		(
			&pool->free_head,
			&task->elem,
			&pool->free_tail
		);

		long_lock_unlock(&pool->pool_lock);
	}
}

static struct pool_thread* new_pool_thread(struct worker_pool* pool)
{
	struct pool_thread* ret =
		(struct pool_thread*) malloc_zero(sizeof(struct pool_thread));

	rrt_init(&(ret->thread));
	ret->pool = pool;
	ret->thread.on_release_callback = on_release;
	//TODO this may leak if thread can not be started, create a teastcase for
	//this, novaprova can "replace" rrt_ start with mocking
	int status = rrt_start(&(ret->thread));
	if(0 != status)
	{
		rrt_destroy_thread(&ret->thread);
		//TEST_ASSERT_EQUAL(0, status);//will fail and show the status
		return NULL;
	}
	return ret;
}

static void worker_pool_exec_function(struct pool_thread* task)
{
	task->executor(task->param);
}

//__attribute__((warn_unused_result));
int wp_submit_task(struct worker_pool* wp, void (*func)(void*), void* param)
{
	long_lock_lock(&wp->pool_lock);

	{
		if(wp_available != wp->status)
		{
			long_lock_unlock(&wp->pool_lock);
			return EBADR;
		}
	}

	struct pool_thread* use =
		(struct pool_thread*) queue_pop_tail_element(&wp->free_head, &wp->free_tail);

	if(NULL == use)
	{
		use = new_pool_thread(wp);
		if(NULL == use)
		{
			long_lock_unlock(&wp->pool_lock);
			return EBUSY;
		}
	}

	queue_add_element(&wp->busy_head, &use->elem, &wp->busy_tail);

	use->executor = func;
	use->param = param;

	//printf("wp_submit_task, thread status: %d, pool_thread ptr: %p\n", rrt_get_state(&use->thread), use);
	//it's must be free
	bool ret = rrt_try_rerun_if_free
	(
		&(use->thread),
		(void (*)(void*)) worker_pool_exec_function,
		(void*) use
	);

	long_lock_unlock(&wp->pool_lock);

	return ret?0:EBUSY;
}

enum worker_pool_status wp_get_status(struct worker_pool* wp)
{
	enum worker_pool_status status;
	long_lock_lock(&wp->pool_lock);
	status = wp->status;
	long_lock_unlock(&wp->pool_lock);
	return status;
}

static void shutdown_all(struct pool_thread* el)
{
	while(NULL != el)
	{
		//TEST_ASSERT_EQUAL(0,
				rrt_graceful_shutdown(&el->thread)
		//)
		;
		el = (struct pool_thread*) el->elem.next;
	}
}

int wp_shutdown(struct worker_pool* pool)
{
	long_lock_lock(&pool->pool_lock);
	if(pool->status == wp_available)
	{
		pool->status = wp_shutting_down;
		shutdown_all((struct pool_thread*) pool->free_head);
		shutdown_all((struct pool_thread*) pool->busy_head);

		long_lock_unlock(&pool->pool_lock);
		return 0;
	}
	else
	{
		long_lock_unlock(&pool->pool_lock);
		return EBADR;
	}
}

static bool is_all_exited(struct pool_thread* el)
{
	while(NULL != el)
	{
		if(rrt_exited != rrt_get_state(&el->thread))
		{
			return false;
		}
		el = (struct pool_thread*) el->elem.next;
	}
	return true;
}

int wp_wait_exit(struct worker_pool* pool)
{
	long_lock_lock(&pool->pool_lock);
	if(pool->status != wp_shutting_down)
	{
		long_lock_unlock(&pool->pool_lock);
		return EBUSY;
	}

	pool->status = wp_exited;

	//go through all the busy and free list, if we still have not exited threads
	//we have to unlock the pool's lock to running threads can access these lists
	//then wait for a while.
	bool exited = false;
	for(;;)
	{
		exited =
			is_all_exited((struct pool_thread*) pool->busy_head)
		&&
			is_all_exited((struct pool_thread*) pool->free_head);

		long_lock_unlock(&pool->pool_lock);

		if(!exited)
		{
			usleep(100000);
			long_lock_lock(&pool->pool_lock);
		}
		else
		{
			return 0;
		}
	}
}

static void free_all(struct pool_thread* el)
{
	struct pool_thread* prev;
	while(NULL != el)
	{
		prev = el;
		el = (struct pool_thread*) el->elem.next;
		free(prev);
	}
}

int wp_destroy(struct worker_pool* pool)
{
	long_lock_lock(&pool->pool_lock);

	//fail if not exited
	if(pool->status != wp_exited)
	{
		long_lock_unlock(&pool->pool_lock);
		return EBADR;
	}

	free_all((struct pool_thread*) pool->busy_head);
	free_all((struct pool_thread*) pool->free_head);

	long_lock_unlock(&pool->pool_lock);
	long_lock_destroy(&pool->pool_lock);

	return 0;
}

int wp_init(struct worker_pool* pool)
{
	memset(pool, 0, sizeof(struct worker_pool));
	pool->status = wp_available;
	return long_lock_init(&pool->pool_lock);
}
