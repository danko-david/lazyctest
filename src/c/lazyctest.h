
#ifndef LAZYCTEST_H
#define LAZYCTEST_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __GLIBC__
	size_t backtrace(void**, size_t);
	char** backtrace_symbols(void**, size_t);
#endif

void render_stack_strace(char* dst, size_t max_size);

#ifndef UTILS_H_
struct queue_element
{
	struct queue_element* prev;
	struct queue_element* next;
};
#endif

enum lct_test_status_flag
{
	LCT_TEST_PARENT_CHECKPOINT,
	LCT_TEST_CHILD_CHECKPOINT,

	LCT_TEST_BEFORE_TEST_FUNCTION,
	LCT_TEST_AFTER_TEST_FUNCTION,


	LCT_TEST_PARENT_RETURN,

	LCT_TEST_CHILD_SIGSEGV,
	LCT_TEST_ASSERT_ERROR,
};

enum lct_test_evaluation_result
{
	LCT_RESULT_INCOMPLETE,
	LCT_RESULT_SUCCESS,
	LCT_RESULT_FAILURE,
	LCT_RESULT_INTERNAL_ERROR
};

enum lct_test_except_type
{
	LCT_EXCEPT_VOID_RETURN,
	LCT_EXCEPT_EXIT_STATUS,
	LCT_EXCEPT_SIGSEGV,
};

const char* lct_fetch_expectation(enum lct_test_except_type);

const char* lct_fetch_test_evaluation_result(enum lct_test_evaluation_result);

struct elf_symbol
{
	struct queue_element as_queue_element;
	const char* unit;
	const char* name;
	const void* address;
};

struct test_job
{
	struct queue_element as_queue_element;

	uint64_t verify_pattern0;

	struct elf_symbol* elf;

	time_t test_start_time;
	time_t test_end_time;

	pid_t child_pid;

	enum lct_test_except_type excepted_test_outcome;
	int excepted_exit_status;

	uint32_t test_status_flags;

	uint64_t verify_pattern1;

	char test_txt_result[2048];

	int proc_exit;

	enum lct_test_evaluation_result test_evaluation_result;

	uint64_t verify_pattern2;
};

extern struct test_job* LCT_CURRENT_TEST_JOB;

void lct_ensure_test_environment();

void lct_append_result_text(char* msg);

void lct_append_stack_strace();

void lct_set_status_flag(struct test_job* job, enum lct_test_status_flag);

void lct_test_pass();

void print_stack_trace();

time_t get_current_time_milisec();

void lct_test_failed
(
	const char *file,
	int line,
	const char *fmt,
	...
);

void lct_test_set_excepted_void_return();
void lct_test_set_excepted_exit_status(int excepted_exit_status);
void lct_test_set_excepted_sigsegv();


void lct_free_regex();
void lct_free_tasks();
void lct_close_dls();

/****************************** source: novaprova *****************************/

#define TEST_ASSERT(cc) \
    do { \
	if (!(cc)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT(" #cc ")"); \
    } while(0)

#define TEST_ASSERT_TRUE(a) \
    do { \
	bool _a = (a); \
	if (!_a) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_TRUE(" #a "=%u)", _a); \
    } while(0)

#define TEST_ASSERT_FALSE(a) \
    do { \
	bool _a = (a); \
	if (_a) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_FALSE(" #a "=%u)", _a); \
    } while(0)

#define TEST_ASSERT_EQUAL(a, b) \
    do { \
	long long _a = (a), _b = (b); \
	if (!(_a == _b)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_EQUAL(" #a "=%lld, " #b "=%lld)", _a, _b); \
    } while(0)

#define TEST_ASSERT_NOT_EQUAL(a, b) \
    do { \
	long long _a = (a), _b = (b); \
	if (!(_a != _b)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_NOT_EQUAL(" #a "=%lld, " #b "=%lld)", _a, _b); \
    } while(0)

#define TEST_ASSERT_PTR_EQUAL(a, b) \
    do { \
	const void *_a = (a), *_b = (b); \
	if (!(_a == _b)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_PTR_EQUAL(" #a "=%p, " #b "=%p)", _a, _b); \
    } while(0)

#define TEST_ASSERT_PTR_NOT_EQUAL(a, b) \
    do { \
	const void *_a = (a), *_b = (b); \
	if (!(_a != _b)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_PTR_NOT_EQUAL(" #a "=%p, " #b "=%p)", _a, _b); \
    } while(0)

#define TEST_ASSERT_NULL(a) \
    do { \
	const void *_a = (a); \
	if (!(_a == NULL)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_NULL(" #a "=%p)", _a); \
    } while(0)

#define TEST_ASSERT_NOT_NULL(a) \
    do { \
	const void *_a = (a); \
	if (!(_a != NULL)) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_NOT_NULL(" #a "=%p)", _a); \
    } while(0)

#define TEST_ASSERT_STR_EQUAL(a, b) \
    do { \
	const char *_a = (a), *_b = (b); \
	if (strcmp(_a ? _a : "", _b ? _b : "")) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_STR_EQUAL(" #a "=\"%s\", " #b "=\"%s\")", _a, _b); \
    } while(0)

#define TEST_ASSERT_STR_NOT_EQUAL(a, b) \
    do { \
	const char *_a = (a), *_b = (b); \
	if (!strcmp(_a ? _a : "", _b ? _b : "")) \
	    lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_STR_NOT_EQUAL(" #a "=\"%s\", " #b "=\"%s\")", _a, _b); \
    } while(0)


#define TEST_ASSERT_STR_EQUAL(a, b) \
    do { \
	const char *_a = (a), *_b = (b); \
	if (strcmp(_a ? _a : "", _b ? _b : "")) \
		lct_test_failed(__FILE__, __LINE__, \
	    "TEST_ASSERT_STR_EQUAL(" #a "=\"%s\", " #b "=\"%s\")", _a, _b); \
    } while(0)

#endif

#ifdef __cplusplus
}
#endif
