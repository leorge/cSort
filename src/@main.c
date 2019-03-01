/*
 ============================================================================
 Name        : @main.c
 Author      : Takeuchi Leorge <qmisort@gmail.com>
 Version     : 1.0.0
 Copyright   : free
 Description : Test a sorting algorithm to compare to sort(1).
 ============================================================================

 Compile option
     -O3 -Wall -c -fmessage-length=0 -std=c99

 Link option
     -lm
 */

#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLOCK_TYPE  CLOCK_PROCESS_CPUTIME_ID
#define OUT     stdout
#define ERR     stderr
#define FALSE   0
#define TRUE    (! FALSE)

/* Functions to test    */
extern void asymm_qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
extern void hole_qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
extern void tag_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
extern void ticket_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));

static void not_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)){}

/* for glibc */
#ifndef __compar_d_fn_t
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);
#endif

// You can edit this function for other data structure.
int     length_compare;
static int cmpstring(const void *p1, const void *p2)    // Function to compare
{
    int rtn = strncmp(p1, p2, length_compare);
    return  rtn;
}

// Measure CPU time

static void cputime(struct timespec *process_now) {
    clock_getres(CLOCK_TYPE, process_now);
    clock_gettime(CLOCK_TYPE, process_now);
}

/*  Algorithm table */

typedef enum {
    DUMMY_SORT,
    QSORT3,
    HOLE,
    ASYMMETRIC,
    TAG_SORT,
    TICKET_SORT,
} SORT_TYPE;

typedef struct {
    int         option;             // command option
    SORT_TYPE   type;               // see above.
    const char *name;               // function name to display
    void        (*sort_function)();
    const char  *description;
} INFO;

static int  cmp_info(const void *t1, const void *t2) {  // to sort test[] in main().
    return ((INFO*)t1)->type - ((INFO*)t2)->type;
}

/*****************************************************************************/
int main(int argc, char *argv[])
{
    extern int getopt(int argc, char * const argv[], const char *optstring);
    extern  int optind;
    extern  char *optarg;
    INFO *info, test[] = {  // alphabetic order in symbol names of enum for each block.
            // aray sort.
            {'3', QSORT3, "qsort(3)", qsort, "qsort(3) in GNU C library."},
            {'a', ASYMMETRIC, "asymm_qsort()", asymm_qsort, "Asymmetric quickSort."},
            {'g', TAG_SORT, "tag_sort()", tag_sort, "taG sort."},
            {'k', TICKET_SORT, "ticket_sort()", ticket_sort, "ticKet sort."},
            {'h', HOLE, "hole_qsort()", hole_qsort, "Simplest new quickSort with a pivot Hole."},
            {'u', DUMMY_SORT, "not_sort()", not_sort, "Not Sort."},
    };

    // prepare to analyze command arguments
    qsort(test, sizeof(test) / sizeof(INFO), sizeof(INFO), cmp_info);   // sort a table according to the SORT_TYPE.
    char    *p, optstring[sizeof(test) / sizeof(INFO) + 100];   // enough long
    size_t  i;
    memset(optstring, 0, sizeof(optstring));
    for (info = test, p = optstring, i = 0; i++ < sizeof(test) / sizeof(INFO); info++) *p++ = (char)info->option;
    strcat(optstring, "?N:");
    /**** Analyze command arguments ****/
    char    *prg = strrchr(argv[0], '/') + 1;   // Program name without path
    if (prg == NULL) prg = argv[0];
    char    *fin = NULL;            // file name to input
    typedef long INDEX;
    INDEX   index = 0, idx;
    int     opt;
    size_t  nmemb = 31, size = 0;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case '?':
            printf("Usage : %s [options] [filename]\n", prg);
            for (info = test, i = 0; i++ < sizeof(test) / sizeof(INFO); info++) {
                printf("\t-%c : %s\n", info->option, info->description);
            }
            puts(
                "\n"
                "\t-N : Number of members. (Default : 31)\n"
            );
            return EXIT_SUCCESS;
            break;
        case 'N':
            nmemb = strtoul(optarg, NULL, 0);
            break;
        default:    // select sorting algorithm
            for (info = test, idx = 0; idx < sizeof(test) / sizeof(INFO); idx++, info++) {
                if (info->option == opt) {
                    index |= 1 << idx;
                    break;
                }
            }
            break;
        }
    }
    if (argc > optind) fin = argv[optind];

    /**** Read data ****/

    // Open file
    FILE *fp = fin ? fopen(fin, "r") : stdin;   // open file or use stdin
    if (fp == NULL) {
        if (fin) fprintf(ERR, "Filename = %s\n", fin);
        perror(NULL);       // Output system error message to OUT.
        return EXIT_FAILURE;
    }

    // Read first line to get a record size
    char read_buff[1024];
    if (! fgets(read_buff, sizeof(read_buff) - 1, fp)) return EXIT_SUCCESS; // EOF
    int buflen = strlen(read_buff); // with EOL(End Of Line : LF or CR+LF)
    if (! isprint((int)read_buff[0])) {         // control character CR, LF, TAB etc.
        fprintf(ERR, "Use printable characters.\n");
        return EXIT_FAILURE;
    } else if (size == 0) {
        length_compare = size = buflen + 1;
    } else if (buflen >= size - 1) {
        fprintf(ERR, "Don't use too long data.\n");
        return EXIT_FAILURE;
    }
    else {
        length_compare = size;
    }

    // allocate data area
    char    *srcbuf;          // Work buffer to store all input data
    if ((srcbuf = calloc(nmemb, size)) == NULL)
    {   /* Can't get work area  */
        perror(NULL);
        return EXIT_FAILURE;
    }

    // read remained data
    i = 0;
    p = srcbuf;
    buflen--;
    do {
        char *q;
        if ((q = strrchr(read_buff, '\n')) != NULL) *q = '\0';  // chop LF
        if ((q = strrchr(read_buff, '\r')) != NULL) *q = '\0';  //      CR
        strncpy(p, read_buff, buflen);
        i++;
        if (! fgets(read_buff, sizeof(read_buff), fp)) break;       // EOF
        p += size;
    } while (i < nmemb);
    if (i <= 1) return EXIT_SUCCESS;
    size_t memsize = (nmemb = i) * size;
    srcbuf = realloc(srcbuf, memsize);

    /*** test ***/

    struct timespec from, to;
    srand((unsigned)time(NULL) + (unsigned)clock());
    for (info = test,idx = 1; index != 0; idx <<= 1, info++) {
        if (index & idx) {
            index &= ~idx;  // clear bit
            cputime(&from);
            (*info->sort_function)(srcbuf, nmemb, size, cmpstring);
            cputime(&to);
            break;  // test only one function
        }
    }
    // output result
    for (p = srcbuf, i = 0; i++ < nmemb; p += size) fprintf(OUT, "%s\n", p);
    fflush(OUT);
    fprintf(ERR, "%s %ld\n", info->name,
            (to.tv_sec - from.tv_sec) * 1000000 + (to.tv_nsec - from.tv_nsec) / 1000);
    return EXIT_SUCCESS;
}
