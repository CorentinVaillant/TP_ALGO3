#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

/*<====================>*Macros*<====================>-------*/

#define tabSize __uint8_t

//fprintf(stderr,"cannot alocate %lu bytes\n",size); 

/*<====================>*Structs*<====================>------*/


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

	RNG rng;
}SkipList;

typedef struct s_DoubleLink{
	Node * next;
	Node * previous;
} DoubleLink;

typedef struct s_Node{
	int val;
	tabSize level; //DoubleLink tab size
	DoubleLink *dl_tab;  //DoubleLink tab

}Node;


/*<====================*Utility funcs*====================>*/

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
	node->level = nb_level;
	node->dl_tab = unwrapMalloc(sizeof(DoubleLink)*nb_level);

}

void delete_node(Node* node){
	assert(node!=NULL);
	free(node->dl_tab);
	free(node);
	node =NULL;
}

Node * node_next(Node *node){
	return node->dl_tab[0].next;
}

DoubleLink create_db(Node *next,Node *previous){
	DoubleLink db;
	db.next = next;
	db.previous = previous;
	return db;
}

Node* skiplist_node_at(const SkipList* d, unsigned int i){
	assert(i>=0);
	assert(i<skiplist_size(d));

	Node * curs_pos = d->sentinel;
	for(int i = 0; i<skiplist_size(d);i++){
		curs_pos = node_next(curs_pos);
	}
	return curs_pos;
}


/*<====================*Publics funcs*====================>*/

/* >-------Create and delete funcs-------< */
SkipList* skiplist_create(int nblevels) {
	//ensure continuity
	struct s_Skiplist *list = unwrapMalloc(sizeof(struct s_Skiplist) + sizeof(struct s_Node) + sizeof(struct s_DoubleLink));
	list->size = 0;
	list->sentinel = (Node*)list+1;
	list->sentinel->level = nblevels;
	list->sentinel->dl_tab = (DoubleLink*)(list->sentinel+1);

	for(int i=0;i<nblevels;i++){
		list->sentinel->dl_tab[i].next = list->sentinel;
		list->sentinel->dl_tab[i].previous = list->sentinel;
	}
	list->rng = rng_initialize(0xFFFFF0000FFFF0000,nblevels);

	return list;
}

void skiplist_delete(SkipList** d) {
	Node * curent = node_next((*d)->sentinel);
	while (curent != (*d)->sentinel){
		Node * to_delete = curent;
		curent = node_next(curent);
		
		delete_node(to_delete);
	}

	free(*d);
	*d =NULL;
}

SkipList* skiplist_insert(SkipList* d, int value) {//TODO
	
	Node * new_node = create_node(value,rng_get_value(&d->rng));
	unsigned int size = skiplist_size(d);

	for(int i = 0; i<new_node->level; i++){
		new_node->dl_tab[i].next = d->sentinel;
		d->sentinel->dl_tab[i].previous = new_node;
	}

}

/* >----------Infos funcs----------< */

unsigned int skiplist_size(const SkipList *d){
	return d->size;
}

int skiplist_at(const SkipList* d, unsigned int i){
	return skiplist_node_at(d,i)->val;
}

/* >----------Util funcs----------< */

void skiplist_map(const SkipList* d, ScanOperator f, void *user_data){
	Node * cur_pos = node_next(d->sentinel);
	while (cur_pos != d->sentinel){
		f(cur_pos->val,user_data);
		node_next(cur_pos);
	}
}