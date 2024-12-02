#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "skiplist.h"

#include "rng.h"

/*<====================>*Macros*<====================>-------*/

#define tabSize __uint8_t

//#define DEBUG

#ifdef DEBUG
#define debug_print(...) printf(__VA_ARGS__)
#endif
#ifndef DEBUG
#define debug_print(...)
#endif

/*<====================>*Structs*<====================>------*/

struct s_SkipList{
	unsigned int size;
	struct s_Node* sentinel;

	RNG rng;
} ;

typedef struct s_DoubleLink{
	struct s_Node * next;
	struct s_Node * previous;
} DoubleLink;

typedef struct s_Node{
	int val;
	tabSize level; //DoubleLink tab size
	struct s_DoubleLink *dl_tab;  //DoubleLink tab

}  Node;


struct s_SkipListIterator{
	enum slit_direction direction;
	struct s_SkipList * list;
	struct s_Node * pos;
};


/*<====================*Utility funcs*====================>*/

void * unwrapMalloc(size_t size){
	void * ptr = malloc(size);
	if(!ptr){
		fprintf(stderr,"error : could not allocate %lu bytes of memory",size);
		perror("exiting the program");
		exit(1);
	}
	else return ptr;
}

Node* create_node(int val,tabSize nb_level){

	debug_print("creating node\n");

	Node * node = unwrapMalloc(sizeof(Node));
	node->dl_tab = malloc(sizeof(DoubleLink)*nb_level);
	node->val = val;
	node->level = nb_level;

	debug_print("end creating node\n");

	return node;
}

void delete_node(Node** node){
	debug_print("deleting node\n");

	assert(node!=NULL);
		
	free(*node);
	*node =NULL;
	debug_print("end deleting node\n");
}

Node * node_next(Node *node){
	return node->dl_tab[0].next;
}

Node * node_nth_next_node(Node *node,unsigned int n){
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
	
	debug_print("creating list\n");

	//ensure continuity
	SkipList *list = unwrapMalloc(sizeof(SkipList) );//+ sizeof(struct s_Node) + sizeof(struct s_DoubleLink));
	list->size = 0;
	list->sentinel = unwrapMalloc(sizeof(struct s_Node));//(Node*)list+1;
	list->sentinel->val=0x4B1DE;
	list->sentinel->level = nblevels;
	list->sentinel->dl_tab = unwrapMalloc( sizeof(struct s_DoubleLink)*nblevels) ;//(DoubleLink*)(list->sentinel+1);
	list->rng = rng_initialize(0x4B1DE,nblevels);


	for(int i=0;i<nblevels;i++){
		
		list->sentinel->dl_tab[i].next = list->sentinel;
		list->sentinel->dl_tab[i].previous = list->sentinel;
	}
	debug_print("end creating list\n");

	return (SkipList*)list;
}

void skiplist_delete(SkipList** d) {
	debug_print("deleting list\n");

	Node * curent = node_next((*d)->sentinel);
	while (curent != (*d)->sentinel){
		Node * to_delete = curent;
		curent = node_next(curent);
		
		delete_node(&to_delete);
	}

	free(*d);
	*d =NULL;
}

SkipList* skiplist_insert(SkipList* d, int value) {
	debug_print("inserting\n");

	Node * to_insert_after [d->sentinel->level];
	Node * new_node = create_node(value,rng_get_value(&d->rng)+1);
	Node * cur_pos = d->sentinel;
	Node * next ;

	int level_pos = d->sentinel->level - 1;
	while (level_pos>=0){
		next = cur_pos->dl_tab[level_pos].next;
		if (next == d->sentinel || next->val>value){
			to_insert_after[level_pos]=cur_pos;
			level_pos--;
		}
		else if(next->val < value)
			cur_pos = next;
		else{
			debug_print("clé dupliqué\n");
			return d;
		}

	}

	for (unsigned int i=0; i<new_node->level;++i){
		node_nth_next_node(to_insert_after[i],i)->dl_tab[i].previous = new_node;
		new_node->dl_tab[i].next = node_nth_next_node(to_insert_after[i],i);


		to_insert_after[i] ->dl_tab[i].next = new_node;
		new_node->dl_tab[i].previous = to_insert_after[i];
		
	}
	d->size++;
	debug_print("end inserting\n");

	return d;
}


SkipList* skiplist_remove(SkipList* d, int value){
debug_print("removing %d\n", value);

	Node * to_remove_after [d->sentinel->level];
	Node * cur_pos = d->sentinel;
	Node * next ;

	int level_pos = d->sentinel->level - 1;
	while (level_pos>=0){

		next = cur_pos->dl_tab[level_pos].next;
		if (!(next == d->sentinel || next->val>value)){
			to_remove_after[level_pos]=cur_pos;
			level_pos--;
		}
		else if(!(next->val < value))
			cur_pos = next;
		else{
			debug_print("clé inéxistante\n");
			return d;
		}

	}

	for (unsigned int i=0; i<cur_pos->level;++i){
		node_nth_next_node(to_remove_after[i],i)->dl_tab[i].previous = cur_pos->dl_tab[i].previous;

		to_remove_after[i] ->dl_tab[i].next = cur_pos->dl_tab[i].next;
	}
	d->size--;
	delete_node(&cur_pos);
	debug_print("end removing\n");

	return d;
}
/* >----------Infos funcs----------< */

unsigned int skiplist_size(const SkipList *d){
	return d->size;
}

int skiplist_at(const SkipList* d, unsigned int i){
	return skiplist_node_at(d,i)->val;
}

bool skiplist_search(const SkipList* d, int value, unsigned int *nb_operations) {
    debug_print("Searching value: %d\n", value);
    Node *cur_pos = d->sentinel;  // Début au nœud sentinelle
    int cur_level = d->sentinel->level - 1; // Niveau le plus élevé
    Node *next_node;
	(*nb_operations)++;


    // Recherche dans chaque niveau
    while (cur_level >= 0) {
        next_node = node_nth_next_node(cur_pos, cur_level);

        while (next_node!=d->sentinel && next_node->val < value) {
            cur_pos = next_node;
            (*nb_operations)++;
            next_node = node_nth_next_node(cur_pos, cur_level);
        }

        // Vérification si la valeur est trouvée
        if (next_node!=d->sentinel && next_node->val == value) {
            debug_print("End searching value: %d (found: true) with %u operations\n", value, *nb_operations);
            return true;
        }

        // Descente au niveau inférieur
        cur_level--;
	}
	debug_print("End searching value: %d (found: false) with %u operations\n", value, *nb_operations);
    return false;
}

/* >----------Util funcs----------< */

void skiplist_map(const SkipList* d, ScanOperator f, void *user_data){

	Node * cur_pos = node_next(d->sentinel);
	while (cur_pos != d->sentinel){
		f(cur_pos->val,user_data);
		cur_pos = node_next(cur_pos);
	}
}


/* >----------iterators----------< */
SkipListIterator* skiplist_iterator_create(SkipList* d, IteratorDirection w){
	SkipListIterator * result = unwrapMalloc(sizeof(SkipListIterator));
	result->direction = w;
	result->list = d;
	result->pos = node_next(d->sentinel);
	return result;
}

void skiplist_iterator_delete(SkipListIterator** it){
	free(*it);
	*it = NULL;
}

SkipListIterator* skiplist_iterator_begin(SkipListIterator* it){
	it->pos = it->direction == FORWARD_ITERATOR 
		? it->list->sentinel->dl_tab[0].next 
		: it->list->sentinel->dl_tab[0].previous;

	return it;
}

bool skiplist_iterator_end(SkipListIterator* it){
	return (it->pos == it->list->sentinel);
}

SkipListIterator* skiplist_iterator_next(SkipListIterator* it){
	it->pos = it->direction == FORWARD_ITERATOR 
		? it->pos->dl_tab[0].next
		: it->pos->dl_tab[0].next;
	return it;
}

int skiplist_iterator_value(SkipListIterator* it){
	return it->pos->val;
}