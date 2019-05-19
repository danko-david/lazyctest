/*
 * utils.h
 *
 *  Created on: 2016.02.04.
 *      Author: szupervigyor
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "include.h"

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]

// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)

#ifndef UNUSED
	#define UNUSED(x) (void)(x)
#endif

/*
	Function for manage arrays used in lxc.
	- Generally we initialize an empty structure
		with 0 size and NULL array pointer.

	- Arrays are continuous and terminated with NULL
		if elements are less than array size.
*/

char* pointer_to_string(void*);

void* string_to_pointer(char*);

char* render_string(int maxlen, const char* fmt, void* argptr);

char* copy_string(const char* str);

void* offset_bytes(void* addr, int bytes);

void* malloc_zero(size_t);

void* realloc_zero(void* addr, size_t old_len, size_t new_length);


/**
 * Array types: //TODO testcase for every statement
 * 	array_nt_*
 * 	- null terminated arrays:
 * 		- no intermediate NULL permitted,
 * 		- inserting and appending null has no effect
 *	 	- the end of the array terminated with NULL value.
 * 		- intermediate element removal shifts the array.
 *
 *
 *	array_fix_*
 *		- length of the array tracked, intermediate null permitted, but
 *		- inserting null has no effect.
 *		- removing an intermediate element not shifts the array
 *		=> Useful if you don't want to the array get reallocate on every
 *			element insert/remove.
 *
 *	array_pnt_*
 *	- packed null terminated:
 *		- array terminated with a NULL, no intermediate NULL value permitted.
 *		- resize on every insert/remove. (removing intemedite element shifts
 *			the arrays)
 *
 * */


void array_nt_init(void*** init, uint *len);

/*
 * pop the specified address from the array and shift the elements to the lower
 * index (stay continuous)
 * */
void* array_nt_pop_element(void** array_addr, uint length, uint index);

/**
 * Add element to the end of the array (where NULL found) then terminates the
 * array (if size != length).
 * expands the array if element can't added to.
 * */
int array_nt_append_element(void*** array_addr, uint* length, void* element);

int array_nt_contains(void** array_addr, uint length, void* addr);

int array_nt_population(void** array_addr, uint length);


//void array_nt_ensure_index(void*** array_addr, unsigned int* length, unsigned int index);

void* array_fix_remove_element(void** array_addr, uint length, uint index);

//add element into the first free slot
void array_fix_add_element(void*** array_addr, uint* length, void* element);

bool array_fix_try_add_last_null(void** array_addr, uint length, void* element);

void array_fix_set_element(void*** array_addr, uint* length, uint index, void* element);

void array_fix_ensure_index(void*** array_addr, uint* length, uint index);

int array_fix_contains(void** array_address, uint length, void* val);

int array_fix_population(void** array_address, uint length);

int array_fix_first_free_slot(void** array_address, uint length);


void* array_fix_try_get(void** arr, int length, int index);
/**
 * packed null terminated array
 * an array where NULL not permitted as intermediate value,
 * only place where must occurred is the end of the array.
 * */
void array_pnt_init(void*** array_addr);

int array_pnt_last_index(void** array_addr);

void* array_pnt_pop_element(void*** array_addr, uint index);

int array_pnt_append_element(void*** array_addr, void* element);

int array_pnt_contains(void** array_addr, void* addr);

int array_pnt_population(void** array_addr);

void array_pnt_free_all(void** array_addr);




void array_pnt_dbg_printf_char_array(void** array, char* array_name);

void str_append_continue(char** sb, size_t* sbl, char* str);
int str_append(char** dst, size_t* max_size, char* content);

int safe_strcpy(char* dst, int max_length, const char* src);

int str_post_append(char** dst, size_t* max_size, size_t size);

int gnu_libc_backtrace_symbol(void* addr, char* ret_str, size_t max_length);

void gnu_libc_print_backtraced_symbol(void* addr);

struct array_s
{
	void** arr;
	uint length;
};

typedef struct array_s* array;

array array_create();
void* array_remove_element(array arr, uint index);
void array_add_element(array arr, void* element);
int array_contains(array arr, void* val);
int array_size(array arr);
int array_get_first_free_slot(array arr);
void* array_try_get(array arr, uint index);

struct key_value
{
	void* key;
	void* value;
};

struct queue_element
{
	struct queue_element* prev;
	struct queue_element* next;
};

void queue_add_element
(
	struct queue_element** head,
	struct queue_element* elem,
	struct queue_element** tail
);

struct queue_element* queue_pop_tail_element
(
	struct queue_element** head,
	struct queue_element** tail
);

struct queue_element* queue_pop_head_element
(
	struct queue_element** head,
	struct queue_element** tail
);

void queue_pop_intermediate_element
(
	struct queue_element** head,
	struct queue_element* intermediate,
	struct queue_element** tail
);


struct queue_dll
{
	struct queue_element* head;
	struct queue_element* tail;
};

typedef struct queue_dll* queue;

queue queue_create();

void queue_add_head(queue q, struct queue_element* elem);

struct queue_element* queue_pop_tail(queue q);

struct queue_element* queue_pop_head(queue q);

void queue_pop_intermediate(queue q, struct queue_element* intermediate);


#endif /* UTILS_H_ */
