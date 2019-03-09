/*
 * tag_sort.c
 *
 *  Simple indirect sort.
 *
 *  Created on: 2018/03/01
 *      Author: leo
 */

#include <stdlib.h>
#include <string.h>

#define copy(a, b)  memcpy((a), (b), size)

static int (*comp)(const void *, const void *);
static int my_comp(const void *p1, const void *p2) {
    int rtn = comp(*(const void **)p1,  *(const void **)p2);
    if (! rtn) rtn = *(const void **)p1 > *(const void **)p2? 1 : -1;	// for stability
    return	rtn;
}

void pointer_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
    void    **tags = calloc(sizeof(void *), nmemb); // Allocate an index.
    if ( ! tags)   // failed to allocate memory
        qsort(base, nmemb, size, compare);
    else {
        comp = compare;
        char *src = base;
        void **tag = tags;
        for (size_t i = 0; i < nmemb; i++) {   // Make an index.
            *tag++ = src; src += size;
        }
        qsort(tags, nmemb, sizeof(void *), my_comp); // Sort the index
        // reorder array elements
        char save[size];
        void **t, *dst, *idx;
        tag = tags; dst = base;
        for (size_t i = 0; i < nmemb; i++) {
            if (*(t = tag) != dst) {    // an element is not placed at the correct position
                idx = dst; copy(save, idx); // save an element
                do {
                    copy(idx, src = *t);        // move an element
                    *t = idx;                   // reset the address
                    t = &tags[((idx = src) - base) / size];   // points the new hole
                } while (*t != dst);            // cyclic permutation
                copy(*t = src, save);   // restore saved element
            }
            dst += size; tag++;
        }
        free(tags);
    }
}
