
#include "lazyctester.h"

void dump_test_results(struct processing_queues* tad)
{
	struct queue_element* elem = tad->done.queue->head;
	while(NULL != elem)
	{
		struct test_job* test = (struct test_job*) elem;
		printf
		(
			"Test result: %s is %s\nDetails:\n\
-------------------------------------- 8< --------------------------------------\n\
%s\n\
-------------------------------------- 8< --------------------------------------\
\n\n\n",
			test->elf->name,
			lct_fetch_test_evaluation_result(test->test_evaluation_result),
			test->test_txt_result
		);

		elem = elem->next;
	}
}

struct result_group
{
	const char* suite_name;
	int tests;
	int incomplete;
	int success;
	int fail;
	int error;
	array results;
};

static void add_result_to_set(struct result_group* rg, struct test_job* test)
{
	array_add_element(rg->results, test);
	++rg->tests;
	switch(test->test_evaluation_result)
	{
	case LCT_RESULT_SUCCESS: ++rg->success; break;
	case LCT_RESULT_FAILURE: ++rg->fail; break;

	case LCT_RESULT_INCOMPLETE: ++rg->incomplete; break;
	case LCT_RESULT_INTERNAL_ERROR: ++rg->error; break;

	default: ++rg->error; break;
	}
}

static void append_result(array res, struct test_job* test)
{
	size_t size = array_size(res);
	for(size_t i=0;i<size;++i)
	{
		struct result_group* rg = ((struct result_group*) res->arr[i]);
		if
		(
			!strcmp
			(
				test->elf->unit,
				rg->suite_name
			)
		)
		{
			add_result_to_set(rg, test);
			return;
		}
	}

	struct result_group* rg = (struct result_group*) malloc_zero(sizeof(struct result_group));
	rg->results = array_create();
	rg->suite_name = test->elf->unit;
	array_add_element(res, rg);
	add_result_to_set(rg, test);
}

void write_test_result_junit
(
	struct processing_queues* tad,
	void (*append)(void*, const char*),
	void* user_data
)
{
	array rgs = array_create();

	struct queue_element* elem = tad->done.queue->head;
	while(NULL != elem)
	{
		struct test_job* test = (struct test_job*) elem;
		append_result(rgs, test);
		elem = elem->next;
	}

	append(user_data, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testsuites time=\"");

	{
		char dstr[25];
		time_t now = time(NULL);
		struct tm *t = gmtime(&now);
		strftime(dstr, sizeof(dstr)-1, "%Y-%m-%d_%H:%M:%SU", t);
		append(user_data, dstr);
	}
	append(user_data, "\">\n");

	size_t suites = array_size(rgs);
	char buffer[255];

	for(size_t s=0;s<suites;++s)
	{
		struct result_group* rg = rgs->arr[s];
		sprintf
		(
			buffer,
			"\t<testsuite name=\"%s\" tests=\"%d\" started=\"%d\" failures=\"%d\" errors=\"%d\" ignored=\"%d\">\n",
			rg->suite_name,
			rg->tests,
			rg->tests,
			rg->fail,
			rg->error,
			0
		);
		append(user_data, buffer);

		size_t tlen = array_size(rg->results);
		for(size_t t =0;t<tlen;++t)
		{
			struct test_job* tj = (struct test_job*) rg->results->arr[t];

			char expectation[50];
			switch(tj->excepted_test_outcome)
			{
			case LCT_EXCEPT_VOID_RETURN:
				strcpy(expectation, "RETURN_VOID");
				break;
			case LCT_EXCEPT_EXIT_STATUS:
				sprintf(expectation, "EXIT_STATUS:%d", tj->proc_exit);
				break;
			case LCT_EXCEPT_SIGSEGV:
				strcpy(expectation, "SIGSEGV");
				break;
			default:
				sprintf(expectation, "Unknown expectation: %d", tj->excepted_test_outcome);
				break;
			}

			char outcome[50];

			if(tj->test_status_flags & (1 << LCT_TEST_ASSERT_ERROR))
			{
				strcpy(outcome, "ASSERTION_ERROR");
			}
			else if(tj->test_status_flags & (1 << LCT_TEST_CHILD_SIGSEGV))
			{
				strcpy(outcome, "SIGSEGV");
			}
			else if(tj->test_status_flags & (1 << LCT_TEST_AFTER_TEST_FUNCTION))
			{
				strcpy(outcome, "RETURN_VOID");
			}
			else
			{
				sprintf(outcome, "EXIT_STATUS:%d", tj->proc_exit);
			}

			sprintf
			(
				buffer,
				"\t\t<testcase name=\"%s\" time=\"%d.%d\" expectation=\"%s\" outcome=\"%s\" result=\"%s\"", //TODO time
				tj->elf->name,
				(tj->test_end_time - tj->test_start_time) / 1000,
				(tj->test_end_time - tj->test_start_time) % 1000,
				expectation,
				outcome,
				lct_fetch_test_evaluation_result(tj->test_evaluation_result)
			);
			append(user_data, buffer);

			if(LCT_RESULT_SUCCESS == tj->test_evaluation_result)
			{
				append(user_data, "/>\n");
			}
			else
			{
				append(user_data, ">\n");

					append(user_data, "\t\t\t<failure>\n");
						append(user_data, tj->test_txt_result);
					append(user_data, "\t\t\t</failure>\n");
				append(user_data, "\t\t</testcase>\n");
			}
		}

		append(user_data, "\t</testsuite>\n");
	}

	append(user_data, "</testsuites>\n");
}

