# C Header-Only Library for Pairing Heap Priority Queue

This repository provides two versions of the pairing heap implementation:

- **PH_HEAP_V1.h** - Basic implementation, only essential operations available
- **PH_HEAP_V2.h** - Extended version with parent pointer support

## Core Operations
The library provides the following core operations:

```c
void ph_push(PH_HEAP *heap, PH_NODE *node);
void ph_push_raw(PH_HEAP *heap, PH_NODE *node);
void ph_pop(PH_HEAP *heap);
void ph_decrease_root(PH_HEAP *heap);
void ph_merge_heaps(PH_HEAP *dst, PH_HEAP *src);
void ph_destroy_heap(PH_HEAP *heap);
```

## Extended Functions (PHEAP_V2)
```c
void ph_remove_internal(PH_HEAP *heap, PH_NODE *node);
void ph_remove_at(PH_HEAP *heap, PH_NODE *node);
void ph_move_at(PH_HEAP *heap, PH_NODE *dst, PH_NODE *src);
```

## Example Programs
Example programs demonstrating the library:

- **pheap_sort.c** – Sorting numbers using a pairing heap.
- **maze_solver.c** – Pathfinding algorithm using a priority queue.

