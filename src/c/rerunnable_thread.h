/*
 * rerunnable_thread.h
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#ifndef RERUNNABLE_THREAD_H_
#define RERUNNABLE_THREAD_H_

#include "include.h"

#include "concurrency.h"
#include "pthread.h"

enum rerunnable_thread_state
{
	rrt_initalized,
	rrt_idle,
	rrt_busy,

	//shutdown requested, but task may at this time under execution.
	rrt_shutdown_requested,
	rrt_exited
};

enum rrt_callback_point
{
	rrt_right_after_executed,
	rrt_after_become_idle,
};

struct rerunnable_thread
{
	pthread_t thread;

	short_lock rt_lock;

	struct conditional_wait has_job_cw;

	void (*volatile run)(void*);
	void* parameter;

	enum rerunnable_thread_state status;

	void (*on_release_callback)(struct rerunnable_thread*, enum rrt_callback_point point, void (*funct)(void*), void*);
};


void rrt_init(struct rerunnable_thread*);

int rrt_start(struct rerunnable_thread*) __attribute__((warn_unused_result));

bool rrt_is_free(struct rerunnable_thread*);

bool rrt_try_rerun_if_free(struct rerunnable_thread*, void (*function)(void*), void* param);

int rrt_graceful_shutdown(struct rerunnable_thread*);

enum rerunnable_thread_state rrt_get_state(struct rerunnable_thread*);

int rrt_destroy_thread(struct rerunnable_thread* rrt);

int rrt_poll_wait_exit(struct rerunnable_thread*);

#endif /* RERUNNABLE_THREAD_H_ */
