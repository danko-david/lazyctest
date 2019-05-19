/*
 * utils.c
 *
 *  Created on: 2016.02.04.
 *      Author: szupervigyor
 */

#include "utils.h"

char* render_string(int maxlen, const char* fmt, void* argptr)
{
	char* ret = (char*) malloc(sizeof(char)*maxlen);

	int len = snprintf(ret, maxlen, fmt, argptr);

	if(len >= maxlen)
	{
		free(ret);
		return NULL;
	}

	ret = (char*) realloc(ret, strlen(ret)+1);

	return ret;
}

char* pointer_to_string(void* addr)
{
	COMPILE_TIME_ASSERT(sizeof(void*) <= 12);
	return render_string(30,"%p", addr);
}

void* string_to_pointer(char* ptr)
{
	COMPILE_TIME_ASSERT(sizeof(void*) <= sizeof(long int));
	return (void*) strtol(ptr, NULL, 16);
}

char* copy_string(const char* str)
{
	return strdup(str);
}

void* offset_bytes(void* addr, int bytes)
{
	return ((void*)(((char*)addr) + bytes));
}

void* malloc_zero(size_t size)
{
	return calloc(size, 1);
}

void* realloc_zero(void* addr, size_t old_len, size_t new_length)
{
	addr = realloc(addr, new_length);

	int diff = new_length - old_len;
	if(diff > 0)
	{
		memset(offset_bytes(addr, old_len), 0, diff);
	}

	return addr;
}

void array_nt_init(void*** init, uint *len)
{
	*init = (void**) malloc(sizeof(void*)*2);
	memset(*init, 0, sizeof(void*)*2);
	*len = 2;
}

/*
 * pop the specified address from the array and shift the elements to the lower
 * index (stay continuous)
 * */
void* array_nt_pop_element(void** array_addr, unsigned int len, unsigned int index)
{
	if(NULL == array_addr || len <= index)
	{
		return NULL;
	}
	else
	{
		unsigned int i = index;

		void* ret = array_addr[i];

		//shifting the array if least one element on the upper index
		--len;
		//len is now lower, no extra len+1 needed in the next ops.
		while(i < len)
		{
			array_addr[i] = array_addr[i+1];
			++i;
		}

		//set the last element NULL
		//(no duplication on end of the array)
		array_addr[len] = NULL;

		return ret;
	}
}

/**
 * Add element to the and of the array (where NULL found) then terminates the
 * array (if size != length).
 * expands the array if element can't added to.
 * */
int array_nt_append_element(void*** array_addr, unsigned int* length, void* element)
{
	if(NULL == element)
	{
		return -1;
	}

	if(NULL == *array_addr)
	{
		*array_addr = (void**) malloc(sizeof(void*));
		*length = 1;
		(*array_addr)[0] = element;
		return 0;
	}
	else
	{
		int ep = -1;
		int i=0;
		int len = *length;
		while(i < len)
		{
			if(NULL == (*array_addr)[i])
			{
				ep = i;
				break;
			}
			++i;
		}

		if(-1 == ep)
		{
			//all slot are full (we increase the slot by 1)
			*array_addr = (void**) realloc(*array_addr, (len+1)*sizeof(void*));
			*length = len + 1;
			(*array_addr)[len] = element;
			return len;
		}
		else
		{
			//set the very next free slot.
			(*array_addr)[ep] = element;
			//and terminate if slot available after
			if(ep+1 < len)
			{
				(*array_addr)[ep+1] = NULL;
			}
			return ep;
		}
	}
}

int array_nt_contains(void** array_addr, unsigned int length, void* addr)
{
	if(NULL == array_addr)
		return -1;

	uint i = 0;
	while(i < length)
	{
		if(NULL == array_addr[i])
			return -1;

		if(addr == array_addr[i])
			return i;

		++i;
	}
	return -1;
}

int array_nt_population(void** array_addr, unsigned int length)
{
	if(NULL == array_addr)
		return 0;

	uint i;
	for(i= 0;i < length;++i)
	{
		if(NULL == array_addr[i])
		{
			return i;
		}
	}

	return length;
}

void array_ensure_index(void*** array_addr, unsigned int* length, unsigned int index)
{
	if(*length > index)
		return;

	*array_addr = (void**) realloc(*array_addr, sizeof(void*)*(index+1));
	*length = index+1;
}


/************************* methods for fix arrays *****************************/

void* array_fix_remove_element(void** array_addr, uint length, uint index)
{
	if(NULL == array_addr || length <= index)
		return NULL;

	void* ret = array_addr[index];
	array_addr[index] = NULL;
	return ret;
}

void array_fix_add_element(void*** array_addr, uint* length, void* element)
{
	int index = array_fix_first_free_slot(*array_addr, *length);
	if(-1 == index)
	{
		index = *length;
		//growth the array with 1 slot;
		array_fix_ensure_index(array_addr, length, *length);
	}
	(*array_addr)[index] = element;
}

bool array_fix_try_add_last_null(void** array_addr, uint length, void* element)
{
	int index = array_fix_first_free_slot(array_addr, length);
	if(-1 == index)
	{
		return false;
	}

	array_addr[index] = element;
	return true;
}


void array_fix_ensure_index(void*** array_addr, uint* length, uint index)
{
	if(*length > index)
		return;

	uint oldlen = *length;
	*array_addr = (void**) realloc(*array_addr, sizeof(void*)*(index+1));
	memset(&((*array_addr)[oldlen]), 0, (index+1 - oldlen)* sizeof(void*));
	*length = index+1;
}

//returns the index of the first occurrence of item.
//returns -1 if not found
int array_fix_contains(void** array_address, uint length, void* val)
{
	if(NULL == array_address)
		return -1;

	uint i;
	for(i = 0;i<length;++i)
	{
		if(array_address[i] == val)
		{
			return i;
		}
	}

	return -1;
}

int array_fix_population(void** array_address, uint length)
{
	if(NULL == array_address)
		return 0;

	int population = 0;

	uint i;
	for(i = 0;i<length;++i)
	{
		if(NULL != array_address[i])
		{
			++population;
		}
	}

	return population;

}

int array_fix_first_free_slot(void** array_address, uint length)
{
	if(NULL == array_address)
		return -1;

	uint i;
	for(i=0;i<length;++i)
	{
		if(NULL == array_address[i])
		{
			return i;
		}
	}

	return -1;
}

void* array_fix_try_get(void** arr, int length, int index)
{
	if(NULL == arr || index >= length)
	{
		return NULL;
	}

	return arr[index];
}

/**************************** packed null terminated **************************/

void array_pnt_init(void*** array_addr)
{
	if(NULL != *array_addr)
		return;

	(*array_addr) = (void**) malloc(sizeof(void**));
	(*array_addr)[0] = NULL;
}

int array_pnt_last_index(void** array_addr)
{
	//the array is not yet initalized or the first elemenet is the terminator
	if(NULL == array_addr || NULL == array_addr[0])
	{
		return -1;
	}

	uint i = 1;
	for(;;)
	{
		if(NULL == array_addr[i])
		{
			return i-1;
		}
		++i;
	}
	//here the process will die by segfault (if array corrupted)
	return -1;
}


void* array_pnt_pop_element(void*** array_addr, uint index)
{
	int lst = array_pnt_last_index(*array_addr);
	if(lst < (int) index)
	{
		return NULL;
	}

	void* ret = (*array_addr)[index];

	int i;
	for(i = index;i < lst;++i)//i+1 is NULL at the end of the iteration
	{
		(*array_addr)[i] = (*array_addr)[i+1];
	}

	(*array_addr) = (void**) realloc(*array_addr, sizeof(void*)*(lst+2));

	return ret;
}

int array_pnt_append_element(void*** array_addr, void* element)
{
	if(NULL == element)
	{
		return -1;
	}

	int lst = array_pnt_last_index(*array_addr);
	if(lst < 0)
		lst = 0;
	else
		++lst;

	(*array_addr) = (void**) realloc(*array_addr, sizeof(void*)*(lst+2));
	(*array_addr)[lst] = element;
	(*array_addr)[lst+1] = NULL;

	return lst;
}

int array_pnt_contains(void** array_addr, void* addr)
{
	if(NULL == array_addr)
		return -1;

	int i;
	for(i=0;NULL != array_addr[i];++i)
	{
		if(addr == array_addr[i])
		{
			return i;
		}
	}

	return -1;
}

int array_pnt_population(void** array_addr)
{
	return array_pnt_last_index(array_addr)+1;
}

void array_pnt_free_all(void** array_addr)
{
	if(NULL == array_addr)
		return;

	int i;
	for(i=0;NULL != array_addr[i];++i)
	{
		free(array_addr[i]);
	}

	free(array_addr);
}

void array_pnt_dbg_printf_char_array(void** array, char* array_name)
{
	if(NULL == array)
	{
		printf("%s array is null\r\n", array_name);
		return;
	}
	else
		printf("%s array address: %p\r\n", array_name, array);

	char** arr = (char**)array;

	int i;
	for(i=0;NULL != arr[i];++i)
	{
		printf("%s[%d] = \"%s\"\r\n", array_name, i, arr[i]);
	}
}


/****************************** Array fix *************************************/

array array_create()
{
	return (array) malloc_zero(sizeof(struct array_s));
}

void* array_remove_element(array arr, uint index)
{
	return array_fix_remove_element(arr->arr, arr->length, index);
}

void array_add_element(array arr, void* element)
{
	array_fix_add_element(&arr->arr, &arr->length, element);
}

int array_contains(array arr, void* val)
{
	return array_fix_contains(arr->arr, arr->length, val);
}

int array_size(array arr)
{
	return array_fix_population(arr->arr, arr->length);
}

int array_get_first_free_slot(array arr)
{
	return array_fix_first_free_slot(arr->arr, arr->length);
}

void* array_try_get(array arr, uint index)
{
	return array_fix_try_get(arr->arr, arr->length, index);
}

/***********/

void str_append_continue(char** sb, size_t* sbl, char* str)
{
	int len = strlen(str);
	*sbl = *sbl - len;
	*sb = str + len;
}

int str_append(char** dst, size_t* max_size, char* content)
{
	return str_post_append(dst, max_size, safe_strcpy(*dst, *max_size, content));
}

int str_post_append(char** dst, size_t* max_size, size_t size)
{
	*dst = *dst+size;
	*max_size -= size;
	return size;
}

int safe_strcpy(char* dst, int max_length, const char* src)
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


void queue_add_element
(
	struct queue_element** head,
	struct queue_element* elem,
	struct queue_element** tail
)
{
	if(NULL == *head)
	{
		*head = elem;
		elem->prev = NULL;
	}

	elem->prev = *tail;
	elem->next = NULL;
	if(NULL != *tail)
	{
		(*tail)->next = elem;
	}
	*tail = elem;
}

struct queue_element* queue_pop_tail_element
(
	struct queue_element** head,
	struct queue_element** tail
)
{
	//much simple alternative
	struct queue_element* op = *tail;
	if(NULL == op)
	{
		return NULL;
	}
	queue_pop_intermediate_element(head, *tail, tail);
	return op;
}

struct queue_element* queue_pop_head_element
(
	struct queue_element** head,
	struct queue_element** tail
)
{
	struct queue_element* op = *head;
	if(NULL == op)
	{
		return NULL;
	}
	queue_pop_intermediate_element(head, *head, tail);
	return op;
}

void queue_pop_intermediate_element
(
	struct queue_element** head,
	struct queue_element* intermediate,
	struct queue_element** tail
)
{
	struct queue_element* op;

	if(NULL != intermediate->prev)
	{
		//really an intermedaite element
		intermediate->prev->next = intermediate->next;
	}
	else
	{
		//it's the frist node in the dll
		if(NULL != intermediate->next)
		{
			intermediate->next->prev = NULL;
		}
		*head = intermediate->next;
	}

	if(NULL != intermediate->next)
	{
		//really an intermediate element
		intermediate->next->prev = intermediate->prev;
	}
	else
	{
		//it's the tail of the dll
		if(NULL != intermediate->prev)
		{
			intermediate->prev->next = NULL;
		}
		*tail = intermediate->prev;
	}
}

queue queue_create()
{
	return (queue) malloc_zero(sizeof(struct queue_dll));
}

void queue_add_head(queue q, struct queue_element* elem)
{
	return queue_add_element(&q->head, elem, &q->tail);
}

struct queue_element* queue_pop_tail(queue q)
{
	return queue_pop_tail_element(&q->head, &q->tail);
}

struct queue_element* queue_pop_head(queue q)
{
	return queue_pop_head_element(&q->head, &q->tail);
}

void queue_pop_intermediate(queue q, struct queue_element* intermediate)
{
	return queue_pop_intermediate_element(&q->head, intermediate, &q->tail);
}
