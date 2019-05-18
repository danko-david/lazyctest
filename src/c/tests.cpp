
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

}

