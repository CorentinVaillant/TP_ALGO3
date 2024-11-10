#include <stdio.h>
#
#include <stdlib.h>
#include <assert.h>

#include "list.h"

typedef struct s_LinkedElement {
	int value;
	struct s_LinkedElement* previous;
	struct s_LinkedElement* next;
} LinkedElement;

/* Use of a sentinel for implementing the list :
 The sentinel is a LinkedElement* whose next pointer refer always to the head of the list and previous pointer to the tail of the list
 */
struct s_List {
	LinkedElement* sentinel;
	int size;
};

typedef struct s_Sublist{
	LinkedElement * head;
	LinkedElement * tail;
}Sublist;

/*-----------------------------Usefull functions----------------------*/
void * unwrapMalloc(size_t size){
	void * ptr = malloc(size);
	if(!ptr){
		fprintf(stderr,"error : could not initialize a list; alloc failed");
		exit(1);
	}
	else return ptr;
}

/*--------------------Auxiliary functions----------------------------*/
// #region list sort
Sublist list_split(Sublist l){
	Sublist result;
	result.tail = l.tail;
	
	LinkedElement *curpos = l.tail;
	__uint8_t iter = 0;
	while (curpos != l.head){
		curpos = curpos->next;
		if(iter++ %2 == 0)
			result.tail = result.tail->next;
	}
	result.head = result.tail->next;
	return result;
}

Sublist list_merge(Sublist leftlist, Sublist rightlist, OrderFunctor f){
	if(leftlist.head == NULL)
		return rightlist;
	if(rightlist.head==NULL)
		return leftlist;

	Sublist result;

	if (f(leftlist.head->value,rightlist.head->value)){
		result.head = leftlist.head;
		leftlist.head = leftlist.head->next;
		result.tail = list_merge(leftlist,rightlist,f).tail;
	}
	else{
		result.head = rightlist.head;
		rightlist.head = rightlist.head->next;
		result.tail = list_merge(leftlist,rightlist,f).tail;
	}
	return result;
}

Sublist list_mergesort(Sublist l, OrderFunctor f){
	//case where the list is a singleton or empty (NULL, NULL)
	if(l.head == l.tail){
		return l;
	}
	printf("l values :(%d,%d)\n",l.tail->value,l.head->value);
	
	Sublist tmp_list = list_split(l);
	Sublist right;
	Sublist left;

	left.tail = l.tail;
	left.head = tmp_list.head;

	right.tail =  tmp_list.head == tmp_list.tail? NULL : tmp_list.head->next;
	right.head =  tmp_list.head == tmp_list.tail? NULL : l.head;
	
	//sorting the two sublist
	// printf("left values :(%d,%d)\n",left.tail->value,left.head->value);
	// printf("right values :(%d,%d)\n",right.tail->value,right.head->value);
	for(int i=0;i<50000;i++);
	right= list_mergesort(right,f);
	left = list_mergesort(left ,f);

	//merging the two sublist

	return list_merge(left,right,f);
}
// #endregion

/*-----------------------------------------------------------------*/

List* list_create(void) {

	List* l =  unwrapMalloc(sizeof(List)+sizeof(LinkedElement));
	LinkedElement *p_sentinel = (LinkedElement *)(l+1);

	p_sentinel->value = 0xCACA;
	p_sentinel->next = p_sentinel;
	p_sentinel->previous = p_sentinel;
	l->size=0;
	l->sentinel = p_sentinel;

	return l;
}

/*-----------------------------------------------------------------*/

List* list_push_back(List* l, int v) {
	LinkedElement *new_el = unwrapMalloc(sizeof(LinkedElement));
	new_el->value = v;
	new_el->previous = l->sentinel->previous;
	new_el->next = l->sentinel;
	
	l->sentinel->previous = new_el;
	new_el->previous->next = new_el;

	l->size++; 
	return l;
}

/*-----------------------------------------------------------------*/

void list_delete(ptrList* l) {
	while (!list_is_empty(*l)){
		list_pop_back(*l);
	}
	//no need to free the sentinel, they share the same alloc with l
	free(*l);
	*l = NULL;
	
}

/*-----------------------------------------------------------------*/

List* list_push_front(List* l, int v) {
	LinkedElement *new_el = unwrapMalloc(sizeof(LinkedElement));
	new_el->value = v;
	new_el->next = l->sentinel->next;
	new_el->previous = l->sentinel;
	
	l->sentinel->next = new_el;
	new_el->next->previous = new_el;

	l->size++; 
	return l;
}

/*-----------------------------------------------------------------*/

int list_front(const List* l) {
	assert(!list_is_empty(l));

	return l->sentinel->next->value;
}

/*-----------------------------------------------------------------*/

int list_back(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->previous->value;
}

/*-----------------------------------------------------------------*/

List* list_pop_front(List* l) {
	assert(!list_is_empty(l));

	LinkedElement *to_remove = l->sentinel->next;
	l->sentinel->next = to_remove->next;
	to_remove->next->previous = l->sentinel;
	free(to_remove);
	l->size--;
	
	return l;
}

/*-----------------------------------------------------------------*/

List* list_pop_back(List* l){
	assert(!list_is_empty(l));

	LinkedElement *to_remove = l->sentinel->previous;
	l->sentinel->previous = to_remove->previous;
	to_remove->previous->next = l->sentinel;
	free(to_remove);
	l->size--;
	
	return l;
}

/*-----------------------------------------------------------------*/

List* list_insert_at(List* l, int p, int v) {
	assert(p <= l->size);
	LinkedElement *cur_elem = l->sentinel->next;
	LinkedElement *to_add = unwrapMalloc(sizeof(LinkedElement));
	to_add->value = v;
	for(int i = 0 ; i<p ; i++){
		cur_elem = cur_elem->next;
	}

	cur_elem->previous->next = to_add;
	to_add->next = cur_elem;

	to_add->previous = cur_elem->previous;
	cur_elem->previous = to_add;

	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_remove_at(List* l, int p) {
	assert(p < l->size);
	LinkedElement *to_remove = l->sentinel->next;
	for(int i = 0 ; i<p ; i++){
		to_remove = to_remove->next;
	}
	to_remove->next->previous = to_remove->previous;
	to_remove->previous->next = to_remove->next;
	free(to_remove);

	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

int list_at(const List* l, int p) {
	assert(p < l->size);
	LinkedElement *elem = l->sentinel->next;
	for(int i = 0 ; i<p ; i++){
		elem = elem->next;
	}
	return elem->value;
}

/*-----------------------------------------------------------------*/

bool list_is_empty(const List* l) {
	return l->size == 0;
}

/*-----------------------------------------------------------------*/

int list_size(const List* l) {
	return l->size;
}

/*-----------------------------------------------------------------*/

List* list_map(List* l, ListFunctor f, void* environment) {
	LinkedElement * elem=l->sentinel;

	do{

		elem = elem->next;

		elem->value = f(elem->value,environment);
	}while(elem->next!= l->sentinel);

	return l;
}

/*-----------------------------------------------------------------*/

List* list_sort(List* l, OrderFunctor f) {
	Sublist sub_l;
	sub_l.head = l->sentinel->previous;
	sub_l.tail = l->sentinel->next;

	sub_l = list_mergesort(sub_l,f);

	l->sentinel->previous = sub_l.head;
	l->sentinel->next = sub_l.tail;

	return l;
}

