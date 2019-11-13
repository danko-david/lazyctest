
#include "lazyctester.h"

#include <sys/mman.h>

static void invoke(struct elf_symbol* elf)
{
	void(*invoke)() = (void (*)()) elf->address;
	invoke();
}

void lct_append_stack_strace()
{
	lct_ensure_test_environment();
	char* sb;
	size_t sbl = sizeof(LCT_CURRENT_TEST_JOB->test_txt_result);
	str_append_continue(&sb, &sbl, LCT_CURRENT_TEST_JOB->test_txt_result);
	render_stack_strace(sb, sbl);
}


void lct_init_test_job(struct test_job* job)
{
	memset(job, 0 , sizeof(struct test_job));
	job->verify_pattern0 = LCT_VERIFY_PATTERN_0;
	job->verify_pattern1 = LCT_VERIFY_PATTERN_1;
	job->verify_pattern2 = LCT_VERIFY_PATTERN_2;
}

void lct_test_abort_with_stack_trace_then_exit()
{
	lct_ensure_test_environment();
	struct test_job* job = LCT_CURRENT_TEST_JOB;

	printf("lct_test_abort_with_stack_trace_then_exit: REGISTER FLAG\n");
	lct_set_status_flag(LCT_CURRENT_TEST_JOB, LCT_TEST_CHILD_SIGSEGV);

	if(job->excepted_test_outcome != LCT_EXCEPT_SIGSEGV)
	{
		char* sb;
		size_t sbl = sizeof(job->test_txt_result)-1;

		str_append_continue(&sb, &sbl, job->test_txt_result);

		size_t n = snprintf
		(
			sb,
			sbl,
			"Error when executing test function: %s\n",
			job->elf->name
		);
		str_post_append(&sb, &sbl, n);
		render_stack_strace(sb, sbl);
		lct_exit_with_reason(LCT_EXIT_INTERNAL_ERROR);
	}

	lct_append_stack_strace();
	exit(0);
}

void reg_test_sigsegv()
{
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = lct_test_abort_with_stack_trace_then_exit;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        printf("signal handler error");
}

void handle_test_child(struct test_job* job)
{
	lct_set_status_flag(job, LCT_TEST_PARENT_CHECKPOINT);
	int proc_status;
	waitpid(job->child_pid, &proc_status, 0);
	job->test_end_time = get_current_time_milisec();
	job->proc_exit = WEXITSTATUS(proc_status);
	lct_set_status_flag(job, LCT_TEST_PARENT_RETURN);

	switch(job->excepted_test_outcome)
	{
	case LCT_EXCEPT_VOID_RETURN:
		if
		(
				0 == job->proc_exit
			&&
				(job->test_status_flags & (1 << LCT_TEST_AFTER_TEST_FUNCTION))
		)
		{
			job->test_evaluation_result = LCT_RESULT_SUCCESS;
		}
		break;

	case LCT_EXCEPT_EXIT_STATUS:
		if(job->excepted_exit_status == job->proc_exit)
		{
			job->test_evaluation_result = LCT_RESULT_SUCCESS;
		}
		else
		{
			job->test_evaluation_result = LCT_RESULT_FAILURE;
			char* sb;
			size_t sbl = sizeof(job->test_txt_result);
			str_append_continue(&sb, &sbl, job->test_txt_result);
			str_append(&sb, &sbl, "\n");

			int n = sprintf
			(
				sb,
				"Failure: Bad exit status: EXCEPTED: %d, ACTUAL: %d\n.",
				job->excepted_exit_status,
				job->proc_exit
			);
			str_post_append(&sb, &sbl, n);
		}
		break;

	case LCT_EXCEPT_SIGSEGV:
		if(job->test_status_flags & (1 << LCT_TEST_CHILD_SIGSEGV))
		{
			printf("register sigsegv as sccess\n");
			job->test_evaluation_result = LCT_RESULT_SUCCESS;
		}
		else
		{
			job->test_evaluation_result = LCT_RESULT_FAILURE;
			char* sb;
			size_t sbl = sizeof(job->test_txt_result);
			str_append_continue(&sb, &sbl, job->test_txt_result);
			str_append(&sb, &sbl, "\n");

			int n = sprintf
			(
				sb,
				"Failure: Bad exit status: EXCEPTED: %d, ACTUAL: %d\n.",
				job->excepted_exit_status,
				job->proc_exit
			);
			str_post_append(&sb, &sbl, n);
		}
		break;

	default:
		job->test_evaluation_result = LCT_RESULT_INTERNAL_ERROR;
		char* sb;
		size_t sbl = sizeof(job->test_txt_result);
		str_append_continue(&sb, &sbl, job->test_txt_result);

		int n = sprintf
		(
			sb,
			"Internal error: Unknown excepted test outcome: %d\n",
			job->excepted_test_outcome
		);
		str_post_append(&sb, &sbl, n);

		break;
	}

	//control not reached the section after "invoke" in the child process
	if(LCT_RESULT_INCOMPLETE == job->test_evaluation_result)
	{
		job->test_evaluation_result = LCT_RESULT_FAILURE;
		char* sb;
		size_t sbl = sizeof(job->test_txt_result);
		str_append_continue(&sb, &sbl, job->test_txt_result);

		if(job->excepted_exit_status & (1 << LCT_TEST_CHILD_SIGSEGV))
		{
			int n = sprintf
			(
				sb,
				"Child died by unexpected segmentation failure.\n",
				job->proc_exit
			);
			str_post_append(&sb, &sbl, n);
		}
		else if(!(job->test_status_flags & (1 << LCT_TEST_PARENT_RETURN)))
		{
			int n = sprintf
			(
				sb,
				"Child committed unexpected exit with exits status: %d.\n",
				job->proc_exit
			);
			str_post_append(&sb, &sbl, n);
		}
	}
}

void do_test_in_child(struct test_job* job)
{
	LCT_CURRENT_TEST_JOB = job;
	
	lct_free_regex();
	lct_free_tasks();
	
	reg_test_sigsegv();

	job->test_start_time = get_current_time_milisec();

	lct_set_status_flag(job, LCT_TEST_CHILD_CHECKPOINT);

	lct_set_status_flag(job, LCT_TEST_BEFORE_TEST_FUNCTION);
	invoke(job->elf);
	lct_set_status_flag(job, LCT_TEST_AFTER_TEST_FUNCTION);

	char dstr[25];
	time_t now = time(NULL);
	struct tm *t = gmtime(&now);
	strftime(dstr, sizeof(dstr)-1, "%Y-%m-%d_%H:%M:%SU", t);

	snprintf
	(
		job->test_txt_result,
		2040,
		"Successfull invocation and return of void %s(void) in process %d at %s.\nlct: exit(0)",
		job->elf->name,
		getpid(),
		dstr
	);
	
	//unecessary: lct_close_dls();
	exit(0);
}

/**
 * TODO in the child process, close all unrelated SHM region.
 * */
void execute_test_job(struct processing_queues* work, struct elf_symbol* elf)
{
	//TODO mmap fork setup etc
	struct test_job* shm = mmap
	(
		NULL,
		sizeof(struct test_job),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1,
		0
	);

	lct_init_test_job(shm);
	shm->elf = elf;

	lct_set_status_flag(shm, LCT_TEST_PARENT_CHECKPOINT);
	pid_t fret = fork();

	if(fret < 0)
	{
		//error
		perror("fork error");
		snprintf
		(
			shm->test_txt_result,
			2040,
			"Can't fork lct tester process (%d).", getpid()
		);
		shm->test_evaluation_result = LCT_RESULT_INTERNAL_ERROR;
	}
	else if(0 == fret)
	{
		//child process
		do_test_in_child(shm);
		exit(0);
	}
	else
	{
		//parent process
		shm->child_pid = fret;
		handle_test_child(shm);
	}

	ql_push(&work->done, shm);
}
