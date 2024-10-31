/*
 * mm.c
 *
 * Name 1: Zeyu Li
 * PSU ID 1: zkl5329
 *
 * Name 2: [FILL IN]
 * PSU ID 2: [FILL IN]
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * Also, read malloclab.pdf carefully and in its entirety before beginning.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

 /*
  * If you want to enable your debugging output and heap checker code,
  * uncomment the following line. Be sure not to have debugging enabled
  * in your final submission.
  */
  // #define DEBUG

#ifdef DEBUG
/* When debugging is enabled, the underlying functions get called */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated */
#define dbg_printf(...)
#define dbg_assert(...)
#endif /* DEBUG */

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* DRIVER */

/* What is the correct alignment? */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x + ALIGNMENT - 1) / ALIGNMENT);
}
//aligned to 16 bytes a block.
#define BLOCKSIZE  16

//the heap start address.
void* heap_start_add;
//the heap head end address.
void* heap_head_end_add;
//the empty space start address.
void* space_curr_head_add;
//how much space in the room of now heap
int mallocst_list_num = 0;//how much malloc block we have now.

struct newmallocst
{
    int the_list_num_of_part;//number of the order mallocst. if 0 is empty
    void* add_begin;//malloc space begin
    void* add_end; //the malloc end address with the pad, tail,and the next malloc number
}the_malloc_list[100];

//give the size and check is there have enough space in the list for add that mallocst in
//return 0 for no space for new malloc block
//return other for which list numvber will be free for new malloc block
int check_the_space_of_the_malloc_list(size_t size)
{
    size_t the_smallest_space_in_list_that_fill_in = 0;
    int the_check_num = 0;
    for (int i = 1; i < mallocst_list_num; i++)
    {
        if (the_malloc_list[i].the_list_num_of_part == 0)
        {
            if (the_check_num == 0)
            {
                the_smallest_space_in_list_that_fill_in = the_smallest_space_in_list_that_fill_in + 16;
            }
            else
            {
                the_smallest_space_in_list_that_fill_in = 16;
                the_check_num = 0;
            }
            if (the_smallest_space_in_list_that_fill_in >= size)
            {
                if (the_smallest_space_in_list_that_fill_in > 16)
                {
                    return (i - ((the_smallest_space_in_list_that_fill_in - 16) / 16));

                }
                else
                {
                    return i;
                }
            }
        }
        else
        {
            the_check_num = 1;
        }

    }
    return 0;

}

bool creat_new_block()
{
    void* new_block = mem_sbrk(BLOCKSIZE);
    if (new_block == (void*)-1)//the heap space not enough
    {
        return false;
    }
    else
    {
        the_malloc_list[mallocst_list_num].the_list_num_of_part = 0;
        the_malloc_list[mallocst_list_num].add_begin = heap_start_add;
        the_malloc_list[mallocst_list_num].add_end = heap_start_add + BLOCKSIZE;
        mallocst_list_num++;
        return true;
    }
}

/*
 * Initialize: returns false on error, true on success.
 */
bool mm_init(void)
{
    /* IMPLEMENT THIS */
    return creat_new_block();
}
/*
 * malloc
 */
void* malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    int size_16 = align(size);//the size that aligned to 16 bytes
    int the_list_num = check_the_space_of_the_malloc_list(size_16);
    if (the_list_num != 0)//if we still have enough space in empty space.
    {
        return the_malloc_list[the_list_num].add_begin;
    }
    else
    {
        if (creat_new_block())
        {
            malloc(size);
        }
        else
        {
            return NULL;
        }
    }
    return NULL;//for safe
}

/*
 * free
 */
void free(void* ptr)
{
    if (ptr != NULL)
    {
        for (int i = 1; i <= mallocst_list_num; i++)
        {
            if (the_malloc_list[i].add_begin == ptr)
            {
                if (the_malloc_list[i].the_list_num_of_part != 0)
                {
                    the_malloc_list[i].the_list_num_of_part = 0;
                }
            }
        }
    }
}

/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    /* IMPLEMENT THIS */
    if (oldptr == NULL)//if ptr is NULL, the call is equivalent to malloc(size)
    {
        return malloc(size);
    }
    else if (size == 0)//if size is equal to zero, the call is equivalent to free(ptr) and NULL is returned
    {
        free(oldptr);
    }
    else//if ptr is not NULL,
    {
        void* the_end_of_old_malloc = NULL;
        int the_num_of_list = 0;
        for (int i = 1; i <= mallocst_list_num; i++)
        {
            if (the_malloc_list[i].add_begin == oldptr)
            {
                the_num_of_list = i;
            }
            if (the_malloc_list[i].the_list_num_of_part == the_num_of_list)
            {
                the_malloc_list[i].the_list_num_of_part = 0;
                the_end_of_old_malloc = the_malloc_list[i].add_end;
            }
        }
        size_t size_of_oldptr = the_end_of_old_malloc - oldptr;
        int the_new_list_num = check_the_space_of_the_malloc_list(size);
        void* the_new_list_add;
        if (the_new_list_num != 0)//if we still have enough space in empty space.
        {
            the_new_list_add = the_malloc_list[the_new_list_num].add_begin;
        }
        else
        {
            while (check_the_space_of_the_malloc_list(size) == 0)
            {
                if (creat_new_block())
                {
                    return NULL;
                }
            }
            the_new_list_num = check_the_space_of_the_malloc_list(size);
            the_new_list_add = the_malloc_list[the_new_list_num].add_begin;
        }
        if (size_of_oldptr > size)
        {
            memcpy(the_new_list_add, oldptr, size);
        }
        else
        {
            memcpy(the_new_list_add, oldptr, size_of_oldptr);
        }
        return the_new_list_add;
    }
    return NULL;//for safe
}

/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ptr;
    size *= nmemb;
    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/*
 * Returns whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Returns whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    size_t ip = (size_t)p;
    return align(ip) == ip;
}

/*
 * mm_checkheap
 */
bool mm_checkheap(int lineno)
{
#ifdef DEBUG
    /* Write code to check heap invariants here */
    /* IMPLEMENT THIS */
    bool test = true;
    for (int j = 2; j < the_list_num; j++)
    {
        if (the_malloc_list[j].add_end != the_malloc_list[j + 1].add_begin)
        {
            test = false;
        }
    }
    return test;
#endif /* DEBUG */
    return true;
}

void main()
{
    mm_init();
    malloc(16);
}