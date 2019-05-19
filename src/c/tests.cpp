
#include <stdio.h>

#include "lazyctest.h"

void test_utils(void)
{
	printf("test utils");
}

class something
{
	public:
	static void test_something(void)
	{
		printf("test\n");
	}
};

extern "C"
{
	void test_find_me()
	{
		printf("test_find_me done\n");
	}

	void test_fail_assert()
	{
		TEST_ASSERT(false);
	}

	void test_fail_strs()
	{
		const char* val = "fail";
		TEST_ASSERT_STR_EQUAL("reference", val);
	}

	void test_sleep_and_exit()
	{
		lct_test_set_excepted_exit_status(0);
		usleep(5000);
		exit(0);
	}

	void test_sigsegv()
	{
		lct_test_set_excepted_sigsegv();
		char* val = NULL;
		val[0] = 10;
		//TEST_ASSERT_EQUAL(0,strcmp((const char*)NULL, "str"));
	}
}

