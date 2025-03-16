#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*	Basic A* algorithm implementation using a pairing heap queue
	Compilation: cc maze_solver.c

	Usage: ./a.out <maze_file.txt>
	Provide a text file containing the maze as a command-line argument.

	Maze File Format:
	wall: 'X'
	open path: ' '
	start point: 'S'
	end point: 'E'	*/

enum flag {
	WALL = 1,
	VISITED = 2,
	TOP = 4,
	OPEN = 8,
	PATH = 16,
};

typedef struct pair {
	unsigned int x, y;
} PAIR;

typedef unsigned int DISTANCE;

/*	heuristic function prototype	*/
typedef DISTANCE (*MH)(PAIR *, PAIR *);

/*  Let’s embed all necessary path data inside PH_NODE	*/
typedef struct cell {
	struct cell *ph_child, *ph_list, *ph_parent;
	DISTANCE fscore, distance;
	struct cell *from;
	unsigned int flag;
} CELL;
typedef CELL PH_NODE;
#define PH_NODE_DEFINED

/*  Using Manhattan distance as the heuristic function */
static DISTANCE
mh(PAIR *point1, PAIR *point2) {
	return abs(point1->x - point2->x) + abs(point1->y - point2->y);
}

/*	Comparator function prototype	*/
typedef int (*PH_CMP)(const PH_NODE *const, const PH_NODE *const);

/*	Comparator function	*/
int
cmp(const PH_NODE *const p1, const PH_NODE *const p2) {
	return (int)p2->fscore - (int)p1->fscore;
}
#define PH_CMP_DEFINED

/*	Main data structure definition as PH_HEAP	*/
typedef struct data {
	PH_NODE *ph_root;
	PH_CMP ph_cmp;
	MH mh;
	PAIR startpoint, endpoint, dimensions;
	CELL *ptr;
	size_t size;
} DATA;
typedef DATA PH_HEAP;
#define PH_HEAP_DEFINED

#include "PHEAP_V2.h"

static void
add(DATA *data, CELL *current, PAIR *cpoint, PAIR *npoint) {

	CELL *neighbour = data->ptr + (npoint->x * data->dimensions.y + npoint->y);

	if(neighbour->flag & (WALL | VISITED)) return;
	
	DISTANCE distance = current->distance + 1;
	if(neighbour->flag & TOP) {
		if(distance >= neighbour->distance) return;
		ph_remove_at(data, neighbour);
	} else neighbour->flag |= TOP;

	neighbour->distance = distance;
	neighbour->fscore = data->mh(&data->endpoint, npoint) + distance;
	neighbour->from = current;
	ph_push_raw(data, neighbour);
}

static void
make_list(DATA *data, CELL *current, PAIR *cpoint) {

	/* left	*/
	if(cpoint->y)
		add(data, current, cpoint, &(PAIR){cpoint->x, cpoint->y - 1});

	/*	right	*/
	if(cpoint->y + 1 < data->dimensions.y)
		add(data, current, cpoint, &(PAIR){cpoint->x, cpoint->y + 1});

	/*	down	*/
	if(cpoint->x + 1 < data->dimensions.x)
		add(data, current, cpoint, &(PAIR){cpoint->x + 1, cpoint->y});

	/* up	*/
	if(cpoint->x)
		add(data, current, cpoint, &(PAIR){cpoint->x - 1, cpoint->y});
}

static void
astar(DATA *data) {

	for(CELL *current; (current = data->ph_root); ) {

		data->ph_root = __ph_pop(data, current);
		current->flag |= VISITED;
		current->flag &= ~TOP;

		PAIR cpoint = {
			.x = (current - data->ptr) / data->dimensions.y,
			.y = (current - data->ptr) % data->dimensions.y,
		};

		if(cpoint.x == data->endpoint.x && cpoint.y == data->endpoint.y)
			break;
		make_list(data, current, &cpoint);
	}
}

/*	Function for parsing the maze file and creating runtime data	*/
#define POW 5
#define CHUNK (1 << POW)
#define MASK (CHUNK - 1)

static int
create_maze(DATA *data, FILE *stream) {

	for(int c, len = 0;;) {

		switch((c = getc(stream))) {
			case EOF: goto outloop;
			case '\n':
				if(len != data->dimensions.y) {
					if(data->dimensions.y) {
						fprintf(stderr, "Invalid maze dimensions\n");
						return EXIT_FAILURE;
					}
					data->dimensions.y = len;
				}
				++data->dimensions.x; len = 0; continue;
			case 'X': c = WALL; break;
			case ' ': c = OPEN; break;
			case 'S': c = OPEN; data->startpoint.y = data->size; break;
			case 'E': c = OPEN; data->endpoint.y = data->size; break;
			default: 
				fprintf(stderr, "Invalid character [%c]\n", c);
				return EXIT_FAILURE;
		};
		++len;

		/*  Allocate more memory if needed  */
		if(! (data->size & MASK)) {
			CELL *tmp = realloc(data->ptr, (data->size + CHUNK) * sizeof(CELL));
			if(! tmp) {
				fprintf(stderr, "Memory allocation failed (realloc)\n");
				return EXIT_FAILURE;
			}
			data->ptr = tmp;
			memset(tmp + data->size, 0, CHUNK * sizeof(CELL));
		}

		data->ptr[data->size].flag = c;
		++data->size;
	}
	outloop:

	/*  Shrink memory to fit actual data size */
	void *tmp = realloc(data->ptr, data->size * sizeof(CELL));
	if(! tmp) {
		fprintf(stderr, "Memory allocation failed (realloc)\n");
		return EXIT_FAILURE;
	}
	data->ptr = tmp;

	return EXIT_SUCCESS;
}

/*	Print result	*/
static void
print_maze(DATA *data) {

	CELL *cell = data->ptr;
	for(unsigned int x = 0; x < data->dimensions.x; ++x) {
		for(unsigned int y = 0; y < data->dimensions.y; ++y) {

			char c = cell->flag;
			if(c & PATH) c = '*';
			else if(c & VISITED) c = '.';
			else if(c & TOP) c = '+';
			else if(c & WALL) c = 'X';
			else if(c & OPEN) c = ' ';
			else c = '?';
			putchar(c);
			++cell;
		}
		putchar('\n');
	}
}

int
main(int argc, char *argv[]) {

	while(--argc > 0) {

		DATA data = {
			.ph_cmp = cmp,
			.mh = mh,
		};

		FILE *file = fopen(argv[argc], "r");
		if(! file) {
			fprintf(stderr, "Cannot open %s\n", argv[argc]);
			continue;
		}
		int err = create_maze(&data, file);
		fclose(file);
		if(err == EXIT_FAILURE) {
			fprintf(stderr, "Cannot load data from %s\n", argv[argc]);
			return EXIT_FAILURE;
		}

		/*	Check startpoint	*/
		if(data.startpoint.y == 0) {
			fprintf(stderr, "No startpoint specified\n");
			return EXIT_FAILURE;
		}
		data.ph_root = data.ptr + data.startpoint.y;
		data.startpoint.x = data.startpoint.y / data.dimensions.y;
		data.startpoint.y %= data.dimensions.y;

		/*	Check endpoint	*/
		if(data.endpoint.y == 0) {
			fprintf(stderr, "No endpoint specified\n");
			return EXIT_FAILURE;
		}
		CELL *endpoint = data.ptr + data.endpoint.y;
		data.endpoint.x = data.endpoint.y / data.dimensions.y;
		data.endpoint.y %= data.dimensions.y;

		astar(&data);
		if(endpoint->flag & VISITED) {
			printf("Found path, distance: %u\n", endpoint->distance);

			/*	Path reconstruction	*/
			do endpoint->flag = PATH;
			while((endpoint = endpoint->from));
		}

		print_maze(&data);

		if(data.ptr)
			free(data.ptr);
	}

	return EXIT_SUCCESS;
}

