///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2022 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission Fall 2022, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Main File:        p3Heap.c
// This File:        p3Heap.c
// Other Files:      N/A
// Semester:         CS 354 Fall 2022
// Instructor:       deppeler
//
// Author:           Cinthya Nguyen
// Email:            cnguyen37@wisc.edu
// CS Login:         cinthya
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   including Peer Mentors, Instructors, and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the ideas and help they provided.
//
// Online sources:   Avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   any information you find.
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     *
     * End Mark:
     *  The end of the available memory is indicated using a size_status of 1.
     *
     * Examples:
     *
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     *
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 */
blockHeader *endMark = NULL;

/*
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding
 *   padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to choose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements
 *       - Update all heap block header(s) and footer(s)
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 * - If a BEST-FIT block found is NOT found, return NULL
 *   Return NULL unable to find and allocate block for desired size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void *balloc(int size) {
    size += 4; // block header
    int i;

    if (size % 8 != 0) { // add padding
        i = size % 8;
        size += (8 - i);
    }

    if (size < 0 || size > alloc_size) {
        return NULL;
    }

    blockHeader *next = NULL;
    blockHeader *split = NULL;
    blockHeader *splitNext = NULL;
    blockHeader *curr = heap_start;
    endMark = (blockHeader *) ((void *) heap_start + alloc_size);
    int payload;
    int splitPayload;

    while (curr->size_status != 1) { // allocate space
        payload = curr->size_status;

        if (curr->size_status & 2) { // prev block is alloc
            payload -= 2;
        }

        if (curr->size_status & 1) { // current block is alloc
            payload -= 1;
            curr = (blockHeader *) ((char *) curr + payload);
            continue;
        }

        // block is free
        next = (blockHeader *) ((char *) curr + payload);
        if (payload < size) { // payload is too small, cont. onto next
            curr = (blockHeader *) ((char *) curr + payload);
            continue;
        } else if (payload == size) { // perfect match
            if (curr->size_status & 2) {
                curr->size_status = size + 2 + 1;
            } else {
                curr->size_status = size + 1;
            }

            if (next->size_status != 1) { // update adjacent block
                next->size_status += 2;
            }

            curr = (blockHeader *) ((char *) curr + 4);
            return curr;
        }

        // block is bigger than payload, store for later
        if (split == NULL) {
            split = curr;
            splitPayload = payload;
        } else if (splitPayload > payload) {
            split = curr;
            splitPayload = payload;
        }
        curr = next;
    }

    if (split != NULL) { // split block
        if (split->size_status & 2) {
            split->size_status = size + 2 + 1;
        } else {
            split->size_status = size + 1;
        }

        splitNext = (blockHeader *) ((char *) split + size);

        if (splitNext->size_status != 1) {
            splitNext->size_status = (splitPayload - size) + 2;
        }

        split = (blockHeader *) ((char *) split + 4);
        return split;
    }

    return NULL;
}

/*
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */
int bfree(void *ptr) {
    blockHeader *hdr = (blockHeader *) ((void *) ptr - 4);
    blockHeader *next = NULL;
    int payload;

    if (ptr == NULL) { // null
        return -1;
    }

    if (((int) ptr) % 8 != 0) { // multiple of 8
        return -1;
    }

    // outside of heap space
    if (((int) ptr < (int) heap_start || (int) ptr > (int) endMark)) {
        return -1;
    }

    // free up block
    if (hdr->size_status & 1) {
        payload = hdr->size_status;

        if (payload & 1) { // block is alloc
            payload -= 1;
            hdr->size_status -= 1; //change a-bit
        }

        if (payload & 2) { // prev block is alloc
            payload -= 2;
        }

        if (hdr->size_status & 2) {
            hdr->size_status = payload + 2;
        } else {
            hdr->size_status = payload;
        }

        next = (blockHeader *) ((void *) hdr + payload);
        next->size_status -= 2; //change p-bit of next block

        return 0;
    }

    return -1;
}

/*
 * Function for traversing heap block list and coalescing all adjacent
 * free blocks.
 *
 * This function is used for delayed coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    blockHeader *curr = heap_start;
    blockHeader *next = NULL;
    int payload;
    int coal = 0; // tracks if free blocks were coalesced

    while (curr->size_status != 1) {
        payload = curr->size_status;

        if (curr->size_status & 2) { // prev block alloc
            payload -= 2;
        }

        if (curr->size_status & 1) { // not free
            payload -= 1;
            curr = (blockHeader *) ((char *) curr + payload);
            continue;
        }

        // curr block is free, check next block
        next = (blockHeader *) ((char *) curr + payload);

        if ((int) next == (int) endMark) { // end of heap
            return coal;
        } else if (next->size_status & 1) { // adjacent block is alloc
            curr = next;
            continue;
        } else {
            curr->size_status += next->size_status; // merge
            coal = 1;
        }

    }

    return 0;
}

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */
int init_heap(int sizeOfRegion) {

    static int allocated_once = 0; //prevent multiple myInit calls

    int pagesize;   // page size
    int padsize;    // size of padding when heap size not a multiple of page size
    void *mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader *end_mark;

    if (0 != allocated_once) {
        fprintf(stderr,
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
                    0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }

    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader *) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader *) ((void *) heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader *) ((void *) heap_start + alloc_size -
                                           4);
    footer->size_status = alloc_size;

    return 0;
}

/*
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts)
 * t_End    : address of the last byte in the block
 * t_Size   : size of the block as stored in the block header
 */
void disp_heap() {

    int counter;
    char status[6];
    char p_status[6];
    char *t_begin = NULL;
    char *t_end = NULL;
    int t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used = -1;

    fprintf(stdout,
            "*********************************** Block List **********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout,
            "---------------------------------------------------------------------------------\n");

    while (current->size_status != 1) {
        t_begin = (char *) current;
        t_size = current->size_status;

        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used)
            used_size += t_size;
        else
            free_size += t_size;

        t_end = t_begin + t_size - 1;

        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status,
                p_status, (unsigned long int) t_begin,
                (unsigned long int) t_end, t_size);

        current = (blockHeader *) ((char *) current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout,
            "---------------------------------------------------------------------------------\n");
    fprintf(stdout,
            "*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout,
            "*********************************************************************************\n");
    fflush(stdout);

    return;
}

// end of myHeap.c (Spring 2022)


