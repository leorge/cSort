/*
 * hole_qsort.c
 *
 *  The simplest new quicksort using a pivot hole instead of swaps.
 *
 *  Created on: 2013/01/01
 *      Author: Takeuchi Leorge <qmisort@gmail.com>
 */

#include <string.h>

#define copy(a, b)  memcpy((a), (b), length)

static int      (*comp)(const void *, const void *);
static size_t   length;

static void sort(void *base, size_t nmemb) {
    if (nmemb <= 1) return;
#define first (char *)base
    char *last = first + (nmemb - 1) * length;  // point the last element
    char pivot[length], *hole; copy(pivot, hole = last);    // save the last element as a pivot
    char *lo = first, *hi = last - length;  // search pointers
    for (; lo < hole; lo += length) {       // outer loop
        if (comp(lo, pivot) > 0) {
            copy(hole, lo); hole = lo;      // exchange a larger element with the hole
            for (; hi > hole; hi -= length) {   // inner loop, symmetric to the outer loop
                if (comp(hi, pivot) < 0) {      // symmetric comparison
                    copy(hole, hi); hole = hi;  // exchange a smaller element with the hole
                }
            }
        }
    }
    copy(hole, pivot);  // restore the pivot
    sort(first, (hole - first) / length);           // smaller elements
    sort(hole + length, (last - hole) / length);    // larger elements
}

void hole_qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *))
{
    length = size; comp = compare;
    sort(base, nmemb);
}
