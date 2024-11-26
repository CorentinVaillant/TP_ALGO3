#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

/*<====================>*Macros*<====================>-------*/

#define tabSize __uint8_t


/*<====================>*Structs*<====================>------*/


/*
nb_level ------->next->next->...->next
.
.
.
1--------------> next->next 
0--------------> next
*/

struct s_DoubleLink;
struct s_Node;

typedef struct s_Node Node;
typedef struct s_DoubleLink DoubleLink;

struct s_Skiplist{
	unsigned int size;
	struct s_Node* sentinel;

	RNG rng;
};


struct s_DoubleLink{
	struct s_Node * next;
	struct s_Node * previous;
} ;

struct s_Node{
	int val;
	tabSize level; //DoubleLink tab size
	DoubleLink *dl_tab;  //DoubleLink tab

};



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

	return node;

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

Node * node_get_nth_next_node(Node *node,unsigned int n){
	assert(n<node->level);
	return node->dl_tab[n].next;
}

DoubleLink create_db(Node *next,Node *previous){
	DoubleLink db;
	db.next = next;
	db.previous = previous;
	return db;
}

Node* skiplist_node_at(const SkipList * d, unsigned int pos){
	assert(pos>=0);
	assert(pos<=skiplist_size(d));

	Node * curs_pos = d->sentinel;
	for(unsigned int i = 0; i<pos;i++){
		curs_pos = node_next(curs_pos);
	}
	return curs_pos;
}


/*<====================*Publics funcs*====================>*/

/* >-------Create and delete funcs-------< */
SkipList* skiplist_create(int nblevels) {
	//ensure continuity
	SkipList *list = unwrapMalloc(sizeof(struct s_Skiplist) + sizeof(struct s_Node) + sizeof(struct s_DoubleLink));
	list->size = 0;
	list->sentinel = (Node*)list+1;
	list->sentinel->level = nblevels;
	list->sentinel->dl_tab = (DoubleLink*)(list->sentinel+1);

	for(int i=0;i<nblevels;i++){
		list->sentinel->dl_tab[i].next = list->sentinel;
		list->sentinel->dl_tab[i].previous = list->sentinel;
	}
	list->rng = rng_initialize(0xFFFFF0000FFFF00,nblevels);

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
	
	Node * new_node = create_node(value,rng_get_value(&d->rng)+1);
	Node * cur_pos = d->sentinel;
	int curseur =0;
	
	while (node_get_nth_next_node(cur_pos,curseur) != d->sentinel && value < node_get_nth_next_node(cur_pos,curseur)->val){
		curseur = (curseur+1)%cur_pos->level;
		cur_pos = curseur == 0 ? node_next(cur_pos) : cur_pos;
	}

	cur_pos = node_get_nth_next_node(cur_pos,curseur);
	for(int i=0;i<new_node->level; i++){
		cur_pos->dl_tab[i].previous->dl_tab[i].next = new_node;
		cur_pos->dl_tab[i].previous = new_node;
	}
	return d;

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