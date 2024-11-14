#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

/*-----------------------------*Macros*------------------------------------*/

#define tabSize __uint8_t

//fprintf(stderr,"cannot alocate %lu bytes\n",size); 

/*-----------------------------*Structs*-----------------------------------*/


/*
nb_level ------->next->next->...->next
.
.
.
1--------------> next->next 
0--------------> next
*/

typedef struct s_Skiplist{
	unsigned int size;
	Node* sentinel;
}SkipList;

typedef struct s_DoubleLink{
	Node * next;
	Node * previous;
} DoubleLink;

typedef struct s_Node{
	int val;
	tabSize dl_tab_size; //DoubleLink tab size
	DoubleLink *dl_tab;  //DoubleLink tab

}Node;


/*-----------------------------*Utility funcs*-----------------------------*/

void * unwrapMalloc(size_t size){
	void * ptr = malloc(size);
	if(!ptr){
		fprintf(stderr,"error : could not initialize a list; alloc failed");
		exit(1);
	}
	else return ptr;
}


Node* create_node(int val,tabSize nb_level){
	Node * node = unwrapMalloc(sizeof(Node));
	node->val = val;
	node->dl_tab_size = nb_level;
	node->dl_tab = unwrapMalloc(sizeof(DoubleLink)*nb_level);

}

void delete_node(Node* node){
	assert(node!=NULL);
	free(node->dl_tab);
	free(node);
	node =NULL;
}

DoubleLink create_db(Node *next,Node *previous){
	DoubleLink db;
	db.next = next;
	db.previous = previous;
	return db;
}



/*-----------------------------*Publics funcs*-----------------------------*/

SkipList* skiplist_create(int nblevels) {
	//ensure continuity
	struct s_Skiplist *list = unwrapMalloc(sizeof(struct s_Skiplist) + sizeof(struct s_Node) + sizeof(struct s_DoubleLink));
	list->size = 0;
	list->sentinel = (Node*)list+1;
	list->sentinel->dl_tab_size = nblevels;
	list->sentinel->dl_tab = (DoubleLink*)list+2;

	for(int i=0;i<nblevels;i++){
		list->sentinel->dl_tab[i].next = list->sentinel;
		list->sentinel->dl_tab[i].previous = list->sentinel;
	}


	return list;
}

void skiplist_delete(SkipList** d) {
	Node * curent = (*d)->sentinel->dl_tab[0].next;
	while (curent != (*d)->sentinel){
		Node * to_delete = curent;
		curent = curent->dl_tab[0].next;
		
		delete_node(to_delete);
	}

	free(*d);
	*d =NULL;
	
}

SkipList* skiplist_insert(SkipList* d, int value) {
	(void)value;
	return d;
}
