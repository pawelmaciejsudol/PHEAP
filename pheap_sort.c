#include <stdio.h>
#include <stdlib.h>

/*	Using pairing heap as a sorting routine
	Compilation: cc pheap_sort.c
	Optional flags:
		WITH_PARENT_PTR - Use PHEAP_V2.h implementation
		WITH_ARG - Use three-argument comparator function
	Usage: ./a.out <file.txt>
	Provide a text file containing the numbers to be sorted.
*/

#ifndef NOPRINT
#define PRINT(...) (printf(__VA_ARGS__))
#else
#define PRINT(...)
#endif

/*  Definition of a pairing heap node */
typedef struct ph_node {
	struct ph_node *ph_list, *ph_child;
#ifdef WITH_PARENT_PTR
	struct ph_node *ph_parent;
#endif
	int key;
} PH_NODE;
#define PH_NODE_DEFINED

#ifdef WITH_ARG
/*  Define PH_ARG as a new type */
typedef int PH_ARG;

/*  Define the comparator function prototype  */
typedef int (*PH_CMP)(PH_ARG *const, const PH_NODE *const, const PH_NODE *const);
#define PH_CMP_DEFINED

/*  Define the heap structure with an additional argument */
typedef struct ph_heap {
	PH_NODE *ph_root;
	PH_CMP ph_cmp;
	PH_ARG ph_arg;
} PH_HEAP;
#define PH_HEAP_DEFINED

/*  Macro to compare two nodes  */
#define PH_ISGREATER(ph_heap, x, y) ((ph_heap)->ph_cmp(&(ph_heap)->ph_arg, (x), (y)) > 0)

/*  Three-argument Comparator function */
static int
ph_cmp(PH_ARG *ph_arg, const PH_NODE *const a, const PH_NODE *const b) {
	++*ph_arg;
	return a->key - b->key;
}
#else
/*  Comparator function (without additional argument) */
static int
ph_cmp(const PH_NODE *const a, const PH_NODE *const b) {
	return a->key - b->key;
}
#endif

/*  Include the appropriate pairing heap implementation */
#ifdef WITH_PARENT_PTR
#include "PHEAP_V2.h"
#else
#include "PHEAP_V1.h"
#endif

/*	Function to load data from file or stdin  */
#include <string.h>
#define POW 5
#define CHUNK (1 << POW)
#define MASK (CHUNK - 1)

static int
load_data(PH_NODE **pptr, FILE *stream, int n) {

	PH_NODE *ptr = *pptr;
	for(int val; fscanf(stream, "%d", &val) != EOF; ptr[n].key = val) {
		if(! (++n & MASK)) {
			ptr = realloc(ptr, (n + CHUNK) * sizeof(PH_NODE));
			if(! ptr) return -1;
			memset(ptr + n, 0, CHUNK * sizeof(PH_NODE));
			*pptr = ptr;
		}
	}
	return n;
}

/*  Function to insert data into the pairing heap */
static int
insert_data(PH_HEAP *heap, PH_NODE *ptr, int n) {

	for(int i = 0; i <= n; ++i) {
		//ph_push_raw(heap, ptr + i);
		ph_push(heap, ptr + i);
	}
}

void
sort_data(PH_HEAP *heap) {

	PH_NODE *root = heap->ph_root;
	if(! root) return;
	PRINT("sorted data: ");

	do PRINT("%d ", root->key);
	while((root = __ph_pop(heap, root)));

	heap->ph_root = NULL;
	PRINT("%c", '\n');
}

int
main(int argc, char *argv[]) {

	PH_HEAP heap = { .ph_cmp = ph_cmp, };
	PH_NODE *data = NULL;
	int n = -1;

	if(argc < 2) {
		puts("Reading input from stdin");
		if((n = load_data(&data, stdin, n)) < 0)
			goto failure;

	} else while(--argc > 0) {
		printf("Reading input from %s\n", argv[argc]);
		FILE *fp =  fopen(argv[argc], "r");
		if(! fp) goto failure;
		n = load_data(&data, fp, n);
		fclose(fp);
		if(n < 0) goto failure;
	}

	insert_data(&heap, data, n);
	sort_data(&heap);

#ifdef WITH_ARG
	printf("Comparision count: %u\n", heap.ph_arg);
#endif

failure:

	if(data) free(data);
	return n >= 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

