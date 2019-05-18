
#include "lazyctest.h"

#ifdef __cplusplus
extern "C"
{
#endif

void lct_set_status_flag(struct test_job* job, enum lct_test_status_flag flag)
{
	job->test_status_flags |= 1 << flag;
}

const char* lct_fetch_test_evaluation_result(enum lct_test_evaluation_result r)
{
	switch(r)
	{
	case LCT_RESULT_INCOMPLETE: return "INCOMPLETE";
	case LCT_RESULT_SUCCESS: return "SUCCESS";
	case LCT_RESULT_FAILUIRE:return "FAILURE";
	case LCT_RESULT_INTERNAL_ERROR:return "INTERNAL ERROR";
	}
	return "Unregistered result type";
}

void lct_ensure_test_environment()
{
	if(NULL == LCT_CURRENT_TEST_JOB)
	{
		printf("This process is not attached to LCT tester parent process. Exiting 1\n");
		exit(1);
	}
}

void render_stack_strace(char* dst, size_t max_size)
{
#ifdef __GLIBC__
	char** sb = &dst;

	void* array[10];
	size_t size;
	char** strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	for(i = 0; i < size; i++)
	{
		str_append(sb, &max_size, "\t");
		str_append(sb, &max_size, strings[i]);
		str_append(sb, &max_size, "\n");
	}

	free (strings);
#else
	printf("//TODO implement `render_stack_strace` on this platform!\n");
#endif
}


void lct_test_failed
(
	const char *file,
	int line,
	const char *fmt,
	...
)
{
	lct_ensure_test_environment();
	struct test_job* job = LCT_CURRENT_TEST_JOB;

	char* dst = job->test_txt_result;
	char** sb = (char**) &dst;
	size_t sbl = sizeof(job->test_txt_result)-1;

	job->test_evaluation_result = LCT_RESULT_FAILUIRE;

	{
		size_t n = snprintf
		(
			*sb,
			sbl,
			"Test failure of %s in file %s on line %d\n",
			job->elf->name,
			file,
			line
		);
		str_post_append(sb, &sbl, n);
	}

	{
		va_list args;
		va_start(args, fmt);
		size_t n = vsnprintf(*sb, sbl, fmt, args);
		str_post_append(sb, &sbl, n);
		va_end(args);
	}

	str_append(sb, &sbl, "\n");

	{
		render_stack_strace(*sb, sbl);
	}
	exit(0);
}

#ifdef __cplusplus
}
#endif
