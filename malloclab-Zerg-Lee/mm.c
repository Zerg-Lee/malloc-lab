/*
 * mm.c
 *
 * Name 1: Zeyu Li
 * PSU ID 1: 943122358
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
#define HEADSIZE 8
#define TAILSIZE 8
#define BOTHSIZE 16
/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x + ALIGNMENT - 1) / ALIGNMENT);
}
void* free_list_lists[11];
const int MAXINDEX = 11;
//static functions define
//nax function for 2 size_t num give the max one
static size_t max_num(size_t x, size_t y)
{
    if (x > y)
    {
        return x;
    }
    else
    {
        return y;
    }
}
//get the data in adress
static size_t get(char* ptr)
{
    return *((size_t*)ptr);
}
//put the data in address
static void put(char* ptr, size_t data)
{
    *((size_t*)ptr) = data;
}
// use this function to conbin the data with 2 address(or int)
static size_t pack(size_t size, size_t alloc)
{
    return size | alloc;
}
//use book to git this function and give size
static size_t get_size(char* ptr)
{
    size_t size = get(ptr) & ~0x7;
    return size;
}
//use book to get this function and give alloc data
static size_t get_alloc(char* ptr)
{
    size_t alloc = get(ptr) & 0x1;
    return alloc;
}

// use this function to get the data head
static char* get_head(char* ptr)
{
    return ptr - HEADSIZE;
}
// use this function to get the data tail
static char* get_tail(char* ptr)
{
    return ptr + get_size(get_head(ptr)) - BOTHSIZE;
}
//get adjacent blocks
static char* next_block(char* ptr)
{
    return ptr + get_size(get_head(ptr));
}
static char* prev_block(char* ptr)
{
    return ptr - get_size(ptr - BOTHSIZE);
}
static char* prev_ptr_addr(char* ptr)
{
    return ptr;
}
static char* next_ptr_addr(char* ptr)
{
    return ptr + TAILSIZE;
}
static char* get_prev_ptr(char* p)
{
    return (char*)get(p);
}
static char* get_next_ptr(char* p)
{
    return (char*)get(p + 8);
}

//save pointer
static void save_ptr(char* ptr, char* data)
{
    put(ptr, (size_t)data);
}
//The heap function
//get the seg_list_index for given size
static int get_list_index(size_t size)
{
    int i = 0;
    if (size <= 32) i = 0;
    else if (size <= 64) i = 1;
    else if (size <= 128) i = 2;
    else if (size <= 256) i = 3;
    else if (size <= 512) i = 4;
    else if (size <= 1024) i = 5;
    else if (size <= 2048) i = 6;
    else if (size <= 4096) i = 7;
    else if (size <= 8192) i = 8;
    else if (size <= 16384) i = 9;
    else i = 10;
    return i;
}

//delete one list from the list
static void delete_list(char* ptr)
{
    size_t size = get_size(get_head(ptr));
    int index = get_list_index(size);
    if (get_prev_ptr(ptr) != NULL)
    {
        if (get_next_ptr(ptr) != NULL)
        {
            save_ptr(next_ptr_addr(get_prev_ptr(ptr)), get_next_ptr(ptr));
            save_ptr(prev_ptr_addr(get_next_ptr(ptr)), get_prev_ptr(ptr));
        }
        else
        {
            save_ptr(next_ptr_addr(get_prev_ptr(ptr)), NULL);
            free_list_lists[index] = get_prev_ptr(ptr);
        }
    }
    else
    {
        if (get_next_ptr(ptr) != NULL)
        {
            save_ptr(prev_ptr_addr(get_next_ptr(ptr)), NULL);
        }
        else
        {
            free_list_lists[index] = NULL;
        }
    }

}
static void insert_blk(char* ptr, size_t size) {

    char* search = ptr;
    char* insert = NULL;
    int index = get_list_index(size);
    search = free_list_lists[index];
    if (index == 6 || index == 5)
    {
        while ((search != NULL) && (size < get_size(get_head(search))))
        {
            insert = search;
            search = get_prev_ptr(search);
        }
        if (search == NULL)
        {
            if (insert != NULL)
            {
                save_ptr(prev_ptr_addr(ptr), NULL);
                save_ptr(next_ptr_addr(ptr), insert);
                save_ptr(prev_ptr_addr(insert), ptr);
            }
            else
            {
                save_ptr(prev_ptr_addr(ptr), NULL);
                save_ptr(next_ptr_addr(ptr), NULL);
                free_list_lists[index] = ptr;
            }
        }
        else
        {
            if (insert != NULL)
            {
                save_ptr(prev_ptr_addr(ptr), search);
                save_ptr(next_ptr_addr(search), ptr);
                save_ptr(next_ptr_addr(ptr), insert);
                save_ptr(prev_ptr_addr(insert), ptr);
            }
            else
            {
                save_ptr(prev_ptr_addr(ptr), search);
                save_ptr(next_ptr_addr(search), ptr);
                save_ptr(next_ptr_addr(ptr), NULL);
                free_list_lists[index] = ptr;
            }
        }
    }
    else
    {
        if (search == NULL)
        {
            save_ptr(prev_ptr_addr(ptr), NULL);
            save_ptr(next_ptr_addr(ptr), NULL);
            free_list_lists[index] = ptr;
        }
        else
        {
            save_ptr(prev_ptr_addr(ptr), search);
            save_ptr(next_ptr_addr(search), ptr);
            save_ptr(next_ptr_addr(ptr), NULL);
            free_list_lists[index] = ptr;
        }
    }

    return;
}

static void* place(void* ptr, size_t size)
{
    size_t blk_size = get_size(get_head(ptr));
    size_t remain = blk_size - size;
    delete_list(ptr);
    if (remain <= (2 * BOTHSIZE))
    {
        put(get_head(ptr), pack(blk_size, 1));
        put(get_tail(ptr), pack(blk_size, 1));
    }
    else
    {
        put(get_head(ptr), pack(size, 1));
        put(get_tail(ptr), pack(size, 1));
        put(get_head(next_block(ptr)), pack(remain, 0));
        put(get_tail(next_block(ptr)), pack(remain, 0));
        insert_blk(next_block(ptr), remain);
    }
    return ptr;
}

static void* realloc_place(void* ptr, size_t size)
{
    size_t oldsize = get_size(get_head(ptr));
    size_t remain = oldsize - size;
    if (remain <= 2 * BOTHSIZE)
    {
        put(get_head(ptr), pack(oldsize, 1));
        put(get_tail(ptr), pack(oldsize, 1));
    }
    else
    {
        put(get_head(ptr), pack(size, 1));
        put(get_tail(ptr), pack(size, 1));
        put(get_head(next_block(ptr)), pack(remain, 0));
        put(get_tail(next_block(ptr)), pack(remain, 0));
        insert_blk(next_block(ptr), remain);
    }

    return ptr;
}

static void* coalesce(char* ptr)
{
    size_t prev_alloc = get_alloc(get_head(prev_block(ptr)));
    size_t next_alloc = get_alloc(get_head(next_block(ptr)));
    size_t size = get_size(get_head(ptr));
    if (prev_alloc && next_alloc)
    { //not free
        return ptr;
    }
    else if (prev_alloc && !next_alloc)
    { //next free
        delete_list(ptr);
        delete_list(next_block(ptr));
        size = size + get_size(get_head(next_block(ptr)));
        put(get_head(ptr), pack(size, 0));
        put(get_tail(ptr), pack(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    { // prev free
        delete_list(ptr);
        delete_list(prev_block(ptr));
        size = size + get_size(get_head(prev_block(ptr)));
        put(get_tail(ptr), pack(size, 0));
        put(get_head(prev_block(ptr)), pack(size, 0));
        ptr = prev_block(ptr);
    }
    else
    {//both free
        delete_list(ptr);
        delete_list(prev_block(ptr));
        delete_list(next_block(ptr));
        size = size + get_size(get_head(prev_block(ptr))) + get_size(get_head(next_block(ptr)));
        put(get_head(prev_block(ptr)), pack(size, 0));
        put(get_tail(next_block(ptr)), pack(size, 0));
        ptr = prev_block(ptr);
    }

    insert_blk(ptr, size);
    return ptr;
}

static void* extend_heap(size_t size)
{
    void* ptr;
    size_t new_size;
    new_size = align(size);
    if ((ptr = mem_sbrk(new_size)) == (void*)-1)
        return NULL;
    put(get_head(ptr), pack(new_size, 0));
    put(get_tail(ptr), pack(new_size, 0));
    put(get_head(next_block(ptr)), pack(0, 1));
    insert_blk(ptr, new_size);
    return coalesce(ptr);
}

/*
 * Initialize: returns false on error, true on success.
 */
bool mm_init(void)
{
    int index;
    char* heap_start;
    for (index = 0; index < MAXINDEX; index++)
    {
        free_list_lists[index] = NULL;
    }
    if ((long)(heap_start = mem_sbrk(4 * 8)) == -1)
        return false;
    put(heap_start, 0);
    put(heap_start + 1 * 8, pack(16, 1));
    put(heap_start + 2 * 8, pack(16, 1));
    put(heap_start + 3 * 8, pack(0, 1));
    if (extend_heap(1 << 6) == NULL)
        return -1;
    return true;
}

/*
 * malloc
 */
void* malloc(size_t size)
{
    /* IMPLEMENT THIS */
    size_t new_size;
    size_t extended;
    size_t search_size;
    void* ptr = NULL;
    int index = 0;
    if (size == 0)
    {
        return NULL;
    }

    if (size <= 16)
    {
        new_size = 32;
    }
    else
    {
        new_size = align(size + 16);
    }
    search_size = new_size;
    index = get_list_index(search_size);
    for (int i = index; i < MAXINDEX; i++)
    {
        ptr = free_list_lists[i];
        while (ptr != NULL)
        {
            if (get_size(get_head(ptr)) >= new_size)
            {
                place(ptr, new_size);
                return ptr;
            }
            ptr = get_prev_ptr(ptr);
        }
    }
    if (ptr == NULL)
    {
        extended = new_size;
        if ((ptr = extend_heap(extended)) == NULL)
            return NULL;
    }
    ptr = place(ptr, new_size);
    return ptr;
}

/*
 * free
 */
void free(void* ptr)
{
    /* IMPLEMENT THIS */
    size_t size = get_size(get_head(ptr));
    put(get_head(ptr), pack(size, 0));
    put(get_tail(ptr), pack(size, 0));
    insert_blk(ptr, size);
    coalesce(ptr);
    return;
}

/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    /* IMPLEMENT THIS */
    size_t old_size = get_size(get_head(oldptr));
    size_t new_size = 0;
    char* new_ptr;
    size_t copy_size = 0;
    if (size == 0)
    {
        mm_free(oldptr);
        return 0;
    }
    else if (oldptr == NULL)
    {
        return malloc(size);
    }
    if (size <= 16)
    {
        new_size = 32;
    }
    else
    {
        new_size = align(size + 16);
    }

    if (old_size == new_size)
    {
        return oldptr;
    }
    else if (old_size > new_size)
    {
        return realloc_place(oldptr, new_size);
    }
    else
    {
        new_ptr = malloc(new_size);
        if (new_ptr == NULL)
        {
            return NULL;
        }
        copy_size = old_size - 16;
        memcpy(new_ptr, oldptr, copy_size);

        mm_free(oldptr);
        return new_ptr;
    }
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
    printf("heap start: %ptr\n", mem_heap_lo());
    printf("heap end: %ptr\n", mem_heap_hi());
    print_free_blk();
    print_alloc_block();
#endif /* DEBUG */
    return true;
}