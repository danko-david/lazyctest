
#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include "lazyctester.h"

struct test_job* LCT_CURRENT_TEST_JOB;

const char* lct_fetch_exit_reason(enum lct_exit_reasons reason)
{
	switch(reason)
	{
		case LCT_EXIT_SUCCESS: return "successfull execution";
		case LCT_EXIT_BAD_CLI_ARG_PARAM: return "wrong cli argument parameter";
		case LCT_EXIT_BAD_SHARED_OBJECT: return "bad shared object";
		case LCT_EXIT_INTERNAL_ERROR: return "LazyCTest internal error";
	};

	return "unknown";
}

void lct_exit_with_reason(enum lct_exit_reasons reason)
{
	printf("lct: exit(%d): %s\n", reason, lct_fetch_exit_reason(reason));
	exit(reason);
}

void lct_exit_with_perror_reason(char* msg, enum lct_exit_reasons reason)
{
	perror(msg);
	lct_exit_with_reason(reason);
}

int lct_print_help_and_exit()
{
	printf("TODO print help\n");
	lct_exit_with_reason(LCT_EXIT_BAD_CLI_ARG_PARAM);
}

array TEST_FUNCTION_MATCHERS;

void lct_sigsegv_print_stack_trace_then_exit()
{
	print_stack_trace();
	lct_exit_with_reason(LCT_EXIT_INTERNAL_ERROR);
}

void reg_sigsegv()
{
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = lct_sigsegv_print_stack_trace_then_exit;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        printf("signal handler error");
}

/**
 * TODO discover C++ static "test_" functions:
 * 	- in the root namespace like: _Z10test_utilsv (void test_utils(void))
 * 	- or in namespace: _ZN9something14test_somethingEv (void something::test_something(void))
 * */
bool is_test_function(const char* name)
{
	uint len = array_size(TEST_FUNCTION_MATCHERS);
	struct regex_matcher matcher;
	for(int i=0;i<len;++i)
	{
		memset(&matcher, 0, sizeof(struct regex_matcher));
		if(0 == regex_match(&matcher, TEST_FUNCTION_MATCHERS->arr[i], name))
		{
			if(matcher.group_count > 0)
			{
				return true;
			}
		}
	}
	return false;
}

void print_discovered_functions(array arr)
{
	uint len = array_size(arr);
	for(int i=0;i<len;++i)
	{
		struct elf_symbol* sym = (struct elf_symbol*) arr->arr[i];
		printf
		(
			"discovered test function: %s :: %s\n",
			sym->unit,
			sym->name
		);
	}
}

int collect_all_function_symbol
(
	const char *libpath,
	const char *libname,
	const char *objname,
	const void *addr,
	const size_t size,
	const symbol_bind binding,
	const symbol_type type,
	void *custom
)
{
	if(type == FUNC_SYMBOL && *objname && is_test_function(objname))
	{
		struct elf_symbol* sym = malloc(sizeof(struct elf_symbol));
		sym->unit = libpath;
		sym->name = objname;
		sym->address = addr;
		queue_add_head((queue) custom, &sym->as_queue_element);
	}

	return 0;
}

static void compile_and_add_regex(array arr, const char* regex)
{
	Regex rx = malloc_zero(sizeof(struct compiled_regex));

	const char* errors[1];
	if(0 != regex_compile(rx, regex, 0, errors))
	{
		printf("Error compiling regex: %s, %s\n", regex, errors[0]);
		lct_exit_with_reason(LCT_EXIT_INTERNAL_ERROR);
	}
	array_add_element(TEST_FUNCTION_MATCHERS, rx);
}

static void init_test_function_matchers()
{
	TEST_FUNCTION_MATCHERS = array_create();
	compile_and_add_regex(TEST_FUNCTION_MATCHERS, "^test_");
	compile_and_add_regex(TEST_FUNCTION_MATCHERS, "__test__");
}

void open_dl_file(const char* dl_file, int verbosity)
{
	if(verbosity > 0)
	{
		printf("lct: dlopen(%s)\n", dl_file);
	}

	char* real_path = realpath(dl_file, NULL);
	const char* open;

	if(NULL == real_path)
	{
		if(verbosity > 0)
		{
			printf("Can't fetch real path: %s\n", dl_file);
		}
		open = dl_file;
	}
	else
	{
		open = real_path;
	}

	if(NULL == dlopen(open, RTLD_NOW | RTLD_GLOBAL))
	{
		printf("Can't open shared object: %s\n", dlerror());
		lct_exit_with_reason(LCT_EXIT_BAD_SHARED_OBJECT);
	}

	free(real_path);
}

void* ql_pop(struct queue_with_lock* qwl)
{
	short_lock_lock(&qwl->lock);
	void* ret = queue_pop_tail(qwl->queue);
	short_lock_unlock(&qwl->lock);
	return ret;
}

void ql_push(struct queue_with_lock* qwl, void* element)
{
	short_lock_lock(&qwl->lock);
	queue_add_head(qwl->queue, element);
	short_lock_unlock(&qwl->lock);
}

void thread_function(void* param)
{
	//printf("new thread\n");
	struct processing_queues* work = (struct processing_queues*) param;

	while(true)
	{
		struct elf_symbol* tf = ql_pop(&work->task);

		//no more function to test
		if(NULL == tf)
		{
			//printf("There's no more test to run\n");
			return;
		}

		execute_test_job(work, tf);
	}
}

static void init_processing_queue(struct processing_queues* q)
{
	memset(q, 0, sizeof(struct processing_queues));

	q->task.queue = queue_create();
	short_lock_init(&q->task.lock);

	q->done.queue = queue_create();
	short_lock_init(&q->done.lock);
}

void print_console(void* user, const char* str)
{
	printf(str);
}

void write_to_file(void* user, const char* str)
{
	write(*((int*)user), str, strlen(str));
}


/**
 * TODO args:
 * 	-c concurrency
 * 	-s list of test so files
 * 	-t list of functions to test
 *	-v verbosity
 *	-o test result output file
 *	-g enable/disable valgrind
 *
 * */

int main(int argc, char **argv)
{
	printf("lct: process stared with pid %d.\n", getpid());
	int concurrency = 1;

	reg_sigsegv();
	init_test_function_matchers();

	for(int i=1;i<argc;++i)
	{
		open_dl_file(argv[i], 0);
	}

	struct processing_queues tad;
	init_processing_queue(&tad);

	symbols(collect_all_function_symbol, tad.task.queue);

	if(1 == concurrency)
	{
		thread_function(&tad);
	}
	else
	{
		struct worker_pool pool;
		wp_init(&pool);
		for(int i=0;i<concurrency;++i)
		{
			wp_submit_task(&pool, thread_function, &tad);
		}
		//printf("wait_exit\n");
		wp_wait_exit(&pool);
		wp_destroy(&pool);
		//printf("exit\n");
	}

	void (*append)(void*, const char*) = print_console;

	int fd;
	{
		char file[50];

		char dstr[25];
		time_t now = time(NULL);
		struct tm *t = gmtime(&now);
		strftime(dstr, sizeof(dstr)-1, "%Y-%m-%d_%H:%M:%SU", t);


		sprintf(file, "lct_result_%s.junit.xml", dstr);
		fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(fd < 0)
		{
			printf("Can't create output file (%s), so print everything to stdout.\n", file);
		}
		else
		{
			append = write_to_file;
		}
	}

	write_test_result_junit(&tad, append, &fd);

	if(fd > -1)
	{
		fsync(fd);
		close(fd);
	}
	lct_exit_with_reason(LCT_EXIT_SUCCESS);
	return 0;
}
