
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
	case LCT_RESULT_FAILURE:return "FAILURE";
	case LCT_RESULT_INTERNAL_ERROR:return "INTERNAL ERROR";
	}
	return "Unregistered result type";
}

void print_stack_trace()
{
#ifdef __GLIBC__
	void* array[10];
	size_t size;
	char** strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	printf("Obtained %zd stack frames.\n", size);

	for(i = 0; i < size; i++)
	{
		printf ("%s\n", strings[i]);
	}

	free (strings);
#else
	printf("//TODO implement `print_stack_trace` on this platform!\n");
#endif
}

const char* lct_fetch_expectation(enum lct_test_except_type type)
{
	switch(type)
	{
	case LCT_EXCEPT_VOID_RETURN: return "RETURN_VOID";
	case LCT_EXCEPT_EXIT_STATUS:return "EXIT_STATUS";
	case LCT_EXCEPT_SIGSEGV: return "SIGSEGV";
	}

	return "Unknown test result expectation.";
}

void lct_ensure_test_environment()
{
	if(NULL == LCT_CURRENT_TEST_JOB)
	{
		printf("This process is not attached to LCT tester parent process. Exiting 1\n");
		print_stack_trace();
		exit(1);
	}
}

/*
void lct_append_result_text(char* msg)
{
	lct_ensure_test_environment();
	char* sb;
	size_t sbl = sizeof(LCT_CURRENT_TEST_JOB->test_txt_result);
	str_append_continue(&sb, &sbl, LCT_CURRENT_TEST_JOB->test_txt_result);
	str_append(&sb, &sbl, msg);
}


static void str_append_continue(char** sb, size_t* sbl, char* str)
{
	int len = strlen(str);
	*sbl = *sbl - len;
	*sb = str + len;
}
*/


#ifndef UTILS_H_
static int safe_strcpy(char* dst, int max_length, const char* src)
{
	if(NULL == src || NULL == dst)
		return 0;

	int len = strlen(src);
	if(len < max_length)
	{
		strcpy(dst,src);
	}
	else
	{
		memcpy(dst,src, max_length-1);
		dst[max_length-1] = '\0';
	}

	return len;
}

static int str_post_append(char** dst, size_t* max_size, size_t size)
{
	*dst = *dst+size;
	*max_size -= size;
	return size;
}

static int str_append(char** dst, size_t* max_size, const char* content)
{
	return str_post_append(dst, max_size, safe_strcpy(*dst, *max_size, content));
}
#endif

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

#include <time.h>

time_t get_current_time_milisec()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (ts.tv_sec * 1000) + (ts.tv_nsec/1000000);
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

	lct_set_status_flag(job, LCT_TEST_ASSERT_ERROR);
	job->test_evaluation_result = LCT_RESULT_FAILURE;


	{
		size_t n = snprintf
		(
			*sb,
			sbl,
			"Test failure in function `%s` in file `%s` on line `%d`\n",
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

void lct_test_set_excepted_void_return()
{
	lct_ensure_test_environment();
	LCT_CURRENT_TEST_JOB->excepted_test_outcome = LCT_EXCEPT_VOID_RETURN;
}

void lct_test_set_excepted_exit_status(int excepted_exit_status)
{
	lct_ensure_test_environment();
	LCT_CURRENT_TEST_JOB->excepted_test_outcome = LCT_EXCEPT_EXIT_STATUS;
	LCT_CURRENT_TEST_JOB->excepted_exit_status = excepted_exit_status;
}

void lct_test_set_excepted_sigsegv()
{
	lct_ensure_test_environment();
	LCT_CURRENT_TEST_JOB->excepted_test_outcome = LCT_EXCEPT_SIGSEGV;
}


#ifdef __cplusplus
}
#endif
