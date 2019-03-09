/*
 * ticket_sort.c
 *
 *  Indirect asymmetric quicksort with User-Defined Index
 *  to make the time complexity to O(n log(n)) when the element size is large.
 *
 *  Sorting key is the first member of an array element up to 16 bytes.
 *
 *  Created on: 2018/06/14
 *      Author: leo
 */

#include "stdlib.h"
#include "string.h"

#define copy(a, b)  memcpy((a), (b), size)

typedef struct {
    void    *key1, *key2;	// sorting key data
    void	*body;			// points an array element
    size_t  position;		// array index for stability
} TICKET;

static int (*comp)(const void *, const void *);
static int my_comp(const void *p1, const void *p2) {
    int rtn = comp(p1, p2);	// You have to modify here to adjust your comparison function.
    if (! rtn) {	// for stability
        rtn = ((TICKET *)p1)->position > ((TICKET *)p2)->position? 1: -1;	// Note: size_t is unsigned.
    }
    return  rtn;
}

void tag_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
    TICKET *tickets = calloc(sizeof(TICKET), nmemb);
    TICKET  *tic = tickets;
    if ( !tic)   // failed to allocate memory
        qsort(base, nmemb, size, compare);
    else {
        comp = compare;
        char    save[size], *body = base;
        for (size_t i = 0; i < nmemb; i++) {    // Build up an index.
            tic->body = body;                   // Point an array element.
            tic->key1 = ((TICKET *)body)->key1; // Copy the first 8 bytes.
            tic->key2 = ((TICKET *)body)->key2; // Copy the next 8 bytes.
            tic->position = i;                  // record number (zero origin)
            tic++; body += size;
        }
        qsort(tickets, nmemb, sizeof(TICKET), my_comp); // Sort the index
        // reorder array elements
        TICKET  *t;
        void    *src = base, *dst;
        tic = tickets; body = base;
        for (size_t i = 0; i < nmemb; i++) {
            if ((t = tic)->body != body) {  // an element is not placed at the correct position
                copy(save, dst = body);         // save an element
                do {
                    copy(dst, src = t->body);   // move an element
                    t->body = dst;              // reset the address
                    t = &tickets[((dst = src) - base) / size];   // points the new hole
                } while (t->body != body);      // cyclic permutation
                copy((t->body = src), save);    // restore saved element
            }
            body += size; tic++;
        }
        free(tickets);
    }
}
