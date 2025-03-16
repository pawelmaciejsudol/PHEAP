#ifndef PHEAP_H
#define PHEAP_H

/*	This file contains an implementation of a pairing heap without a parent pointer.

	available operations:
	ph_push, ph_push_raw, ph_pop, ph_decrease_root, ph_merge_heaps, ph_destroy_heap

	Some definitions can be overridden, define PARAM_DEFINED to indicate a modification	*/

/*	Intrusive PH_NODE structure	*/
#ifndef PH_NODE_DEFINED
typedef struct ph_node {
	struct ph_node *ph_list, *ph_child;
} PH_NODE;
#define PH_NODE_DEFINED
#endif

/*	Default comparator function prototype	*/
#ifndef PH_CMP_DEFINED
typedef int (*PH_CMP)(const PH_NODE *const, const PH_NODE *const);
#define PH_CMP_DEFINED
#endif

/*	Main heap structure that holds a pointer to the root element and 
	the comparator function. This structure can be extended to provide additional
	functionality	*/
#ifndef PH_HEAP_DEFINED
typedef struct ph_heap {
	PH_NODE *ph_root;
	PH_CMP ph_cmp;
} PH_HEAP;
#define PH_HEAP_DEFINED
#endif

/*	Helper macro to retrieve the comparator function from a ph_heap structure	*/
#ifndef PH_GET_CMP
#define PH_GET_CMP(ph_heap) ((ph_heap)->ph_cmp)
#endif

/*	Helper macro to retrieve the destroy function from ph_heap structure
	by default PH_DESTROY is using free	*/
#ifndef PH_DESTROY
#define PH_DESTROY(ph_heap, node) (free(node))
#endif

/*	Macro to control element comparison. By default 
	the pairing heap functions as a min-queue	*/
#ifndef PH_ISGREATER
#define PH_ISGREATER(ph_heap, A, B) (PH_GET_CMP(ph_heap)((A), (B)) < 0)
#endif

/*	By default all functions are marked as static	*/
#ifndef PH_EXPORT
#define PH_EXPORT static
#endif

/*	Internal functions	*/
#ifndef PH_INTERNAL_EXPORT
#define PH_INTERNAL_EXPORT inline static
#endif

PH_INTERNAL_EXPORT PH_NODE *
__ph_push(PH_HEAP *heap, PH_NODE *root, PH_NODE *node) {

	if(PH_ISGREATER(heap, root, node)) {
		node->ph_list = root->ph_child;
		root->ph_child = node;
		return root;
	}
	node->ph_child = root;
	return node;
}

/*	Default push implementation, PH_NODE must be initialized before use	*/
PH_EXPORT void
ph_push(PH_HEAP *heap, PH_NODE *node) {

	PH_NODE *root = heap->ph_root;
	heap->ph_root = root
		? __ph_push(heap, root, node)
		: node;
}

PH_EXPORT PH_NODE *
__ph_push_raw(PH_HEAP *heap, PH_NODE *root, PH_NODE *node) {

	if(PH_ISGREATER(heap, root, node)) {
		node->ph_list = root->ph_child;
		root->ph_child = node;
		node->ph_child = NULL;
		return root;
	}
	root->ph_list = NULL;
	node->ph_child = root;
	return node;
}

/*	Push version that initializes all fields of the structure	*/
PH_EXPORT void
ph_push_raw(PH_HEAP *heap, PH_NODE *node) {

	PH_NODE *root = heap->ph_root;
	if(! root) {
		node->ph_child = NULL;
		heap->ph_root = node;
	} else heap->ph_root = __ph_push_raw(heap, root, node);
}

/*	Two pass merge pairing algorithm	*/
PH_EXPORT PH_NODE *
__ph_extract_list(PH_HEAP *heap, PH_NODE *root) {

	PH_NODE *list = NULL;
	for(;;) {
		PH_NODE *B = root->ph_list;
		if(! B) break;
		PH_NODE *C = B->ph_list;
		if(PH_ISGREATER(heap, B, root)) {
			PH_NODE *tmp = B;
			B = root;
			root = tmp;
		}
		B->ph_list = root->ph_child;
		root->ph_child = B;
		if(! C) break;
		root->ph_list = list;
		list = root;
		root = C;
	}

	while(list) {
		PH_NODE *C = list->ph_list;
		if(PH_ISGREATER(heap, list, root)) {
			PH_NODE *tmp = root;
			root = list;
			list = tmp;
		}
		list->ph_list = root->ph_child;
		root->ph_child = list;
		list = C;
	}

	return root;
}

PH_INTERNAL_EXPORT PH_NODE *
__ph_pop(PH_HEAP *heap, PH_NODE *root) {

	return (root = root->ph_child)
		? __ph_extract_list(heap, root)
		: root;
}

/*	Replaces the root inside the ph_heap structure with the next top element	*/
PH_EXPORT void
ph_pop(PH_HEAP *heap) {

	PH_NODE *root = heap->ph_root;
	if(root) {
		heap->ph_root = (root = root->ph_child)
			? __ph_extract_list(heap, root)
			: root;
	}
}

PH_INTERNAL_EXPORT PH_NODE *
__ph_decrease_root(PH_HEAP *heap, PH_NODE *root) {

	root->ph_list = root->ph_child;
	root->ph_child = NULL;
	return __ph_extract_list(heap, root);
}

/*	Decrease function reconnects the root element after update,
	cannot be used on an empty heap	*/
PH_EXPORT void
ph_decrease_root(PH_HEAP *heap) {

	heap->ph_root = __ph_decrease_root(heap, heap->ph_root);
}

/*	Non recursive function based on DSW algorithm,
	invokes PH_DESTROY on every node in queue	*/
PH_INTERNAL_EXPORT void
__ph_destroy_subheap(PH_HEAP *heap, PH_NODE *node) {

	PH_NODE *list;
	do {
		while((list = node->ph_list)) {
			node->ph_list = list->ph_child;
			list->ph_child = node;
			node = list;
		}
		list = node;
		node = node->ph_child;
		PH_DESTROY(heap, list);
	} while(node);
}

/*	__ph_destroy_subheap front-end function	*/
PH_EXPORT void
ph_destroy_heap(PH_HEAP *heap) {

	PH_NODE *root = heap->ph_root;
	if(! root) return;
	heap->ph_root = root->ph_list = NULL;
	return __ph_destroy_subheap(heap, root);
}

PH_INTERNAL_EXPORT PH_NODE *
__ph_merge(PH_HEAP *heap, PH_NODE *root1, PH_NODE *root2) {

	if(PH_ISGREATER(heap, root2, root1)) {
		PH_NODE *tmp = root2;
		root2 = root1;
		root1 = tmp;
	}
	root2->ph_list = root1->ph_child;
	root1->ph_child = root2;
	return root1;
}

/*	Merges two heaps, result is stored in dst heap structure.
	The function uses a comparator located in the dst heap	*/
PH_EXPORT void
ph_merge_heaps(PH_HEAP *dst, PH_HEAP *src) {

	PH_NODE *root = src->ph_root;
	if(! root) return;
	src->ph_root = NULL;
	PH_NODE *ptr = dst->ph_root;
	dst->ph_root = ptr
		? __ph_merge(dst, root, ptr)
		: root;
}
#endif

