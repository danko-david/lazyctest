
#include "lazyctester.h"

#include <sys/mman.h>

struct test_job* create_job(struct elf_symbol* sym)
{
	//TODO
	return NULL;
}

static void invoke(struct elf_symbol* elf)
{
	void(*invoke)() = (void (*)()) elf->address;
	invoke();
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

	char** sb = (char**) &job->test_txt_result;
	size_t sbl = sizeof(job->test_txt_result)-1;

	job->test_evaluation_result = LCT_RESULT_FAILUIRE;

	{
		size_t n = snprintf
		(
			*sb,
			sbl,
			"Internal error when executing test function: %s\n",
			job->elf->name
		);
		str_post_append(sb, &sbl, n);
	}

	{
		render_stack_strace(*sb, sbl);
	}

	lct_exit_with_reason(LCT_EXIT_INTERNAL_ERROR);
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
	reg_test_sigsegv();

	lct_set_status_flag(job, LCT_TEST_PARENT_CHECKPOINT);
	int proc_status;
	waitpid(job->child_pid, &proc_status, 0);
	job->proc_exit = WEXITSTATUS(proc_status);
	lct_set_status_flag(job, LCT_TEST_PARENT_RETURN);

	if(job->test_status_flags & (1 << LCT_TEST_AFTER_TEST_FUNCTION) && 0 == job->proc_exit)
	{
		job->test_evaluation_result = LCT_RESULT_SUCCESS;
	}
	else
	{
		//control not reached the section after "invoke" in the child process
		if(LCT_RESULT_INCOMPLETE == job->test_evaluation_result)
		{
			job->test_evaluation_result = LCT_RESULT_INTERNAL_ERROR;
			//TODO render err messge
		}
	}
	//TODO evaluate results
}

void do_test_in_child(struct test_job* job)
{
	LCT_CURRENT_TEST_JOB = job;
	lct_set_status_flag(job, LCT_TEST_CHILD_CHECKPOINT);

	lct_set_status_flag(job, LCT_TEST_BEFORE_TEST_FUNCTION);
	invoke(job->elf);
	lct_set_status_flag(job, LCT_TEST_AFTER_TEST_FUNCTION);

	char dstr[100];
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
