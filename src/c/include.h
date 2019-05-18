
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include <stdarg.h>

#ifdef INCLUDE_NOVAPROVA
	#define TEST_ASSERT_EQUAL NP_ASSERT_EQUAL
	#define TEST_ASSERT_PTR_EQUAL NP_ASSERT_PTR_EQUAL
	#define TEST_ASSERT_NOT_NULL NP_ASSERT_NOT_NULL
	#define TEST_ASSERT_TRUE NP_ASSERT_TRUE
	#define TEST_ASSERT_NOT_EQUAL NP_ASSERT_NOT_EQUAL
#elif EMBEDDED_TESTER /*Another tester infrastructure.*/



/*




#else	/ *No tester infrastructure* /
	#define TEST_ASSERT_EQUAL
	#define TEST_ASSERT_PTR_EQUAL
	#define TEST_ASSERT_NOT_NULL
	#define TEST_ASSERT_TRUE
	#define TEST_ASSERT_NOT_EQUAL*/
#endif

