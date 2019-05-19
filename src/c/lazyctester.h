
#ifndef LAZYCTESTER_H
#define LAZYCTESTER_H

#include "include.h"

#include <dlfcn.h>
#include <link.h>
#include <signal.h>

#include "utils.h"
#include "symbols.h"
#include "lazyctest.h"

#include <sys/wait.h>

enum lct_exit_reasons
{
	LCT_EXIT_SUCCESS,
	LCT_EXIT_BAD_CLI_ARG_PARAM,
	LCT_EXIT_BAD_SHARED_OBJECT,
	LCT_EXIT_INTERNAL_ERROR,
};

void print_stack_trace();

const char* lct_fetch_exit_reason(enum lct_exit_reasons reason);

void lct_exit_with_reason(enum lct_exit_reasons reason);

void lct_exit_with_perror_reason(char* msg, enum lct_exit_reasons reason);

struct queue_with_lock
{
	queue queue;
	short_lock lock;
};

struct processing_queues
{
	struct queue_with_lock task;
	struct queue_with_lock done;
};

void ql_push(struct queue_with_lock* qwl, void* element);

//TODO populate random longs

#define LCT_VERIFY_PATTERN_0 0xff;
#define LCT_VERIFY_PATTERN_1 0xab;

#define LCT_VERIFY_PATTERN_2 0xfb;

#endif
