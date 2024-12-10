#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
};



typedef BinarySearchTree *(*AccessFunction)(const BinarySearchTree*);

typedef struct {
    bool watching_right;
    AccessFunction first_op;
    AccessFunction second_op;
}ChildAccessors;
/*------------------------  Util funcs  -----------------------------*/

BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessors access){
    assert(!bstree_empty(x));
    BinarySearchTree *curr ;
    if(!bstree_empty(access.watching_right ? x->right: x->left))
        curr = access.first_op(x);
    else {
        curr = access.second_op(x);
    }
    return curr;
}

bool bstree_is_leaf(const BinarySearchTree* x){
    return !(x->left) && !(x->right);
}

BinarySearchTree* bstree_right_then_diving_left(const BinarySearchTree* x){
    BinarySearchTree *curr;
    curr = x->right;
    while (!bstree_empty(curr->left))
        curr = curr->left;
    return curr;
}

BinarySearchTree* bstree_left_then_diving_right(const BinarySearchTree* x){
    BinarySearchTree *curr;
    curr = x->left;
    while (!bstree_empty(curr->right))
        curr = curr->right;
    return curr;
}

BinarySearchTree* bstree_up_while_is_right_child(const BinarySearchTree* x){
    BinarySearchTree *curr;
    curr = x->parent;
    while (!bstree_empty(curr) && x == curr->right){
        x = curr;
        curr = curr->parent;
    }
    return curr;
}

BinarySearchTree* bstree_up_while_is_left_child(const BinarySearchTree* x){
    BinarySearchTree *curr ;
    curr = x->parent;
    while (!bstree_empty(curr) && x == curr->left){
        x = curr;
        curr = curr->parent;
    }
    return curr;
}



/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    t->key = key;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    if(t) free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_iterative_depth_infix(*t, freenode, NULL);
    *t=NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
	
    BinarySearchTree **curr = t;
    BinarySearchTree *par = NULL;
    while (!bstree_empty(*curr)){
        par = *curr;
        if((*curr)->key > v)
            curr = &((*curr)-> left);
        else 
            curr = &((*curr)->right);
    }
    *curr = bstree_cons(NULL,NULL,v);
    (*curr)->parent = par;

    
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    const BinarySearchTree *curr = t;
    while (!bstree_empty(curr) && bstree_key(curr) != v)
        curr = bstree_key(curr) > v ? curr->left : curr->right;
    
    return curr;
    
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    ChildAccessors access;
    access.watching_right=true;
    access.first_op = bstree_right_then_diving_left;
    access.second_op = bstree_up_while_is_right_child;
    return find_next(x,access);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    ChildAccessors access;
    access.watching_right=false;
    access.first_op = bstree_left_then_diving_right;
    access.second_op = bstree_up_while_is_left_child;
    return find_next(x,access);
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));
    BinarySearchTree* new_from_parent = to->parent;
    BinarySearchTree* new_from_left = to->left;
    BinarySearchTree* new_from_right= to->right;
    BinarySearchTree**new_from_link=tree;
    if(to->parent)
        new_from_link = to->parent->left == to ? &to->parent->left : &to->parent->right;

    BinarySearchTree* new_to_parent = to->parent;
    BinarySearchTree* new_to_left = to->left;
    BinarySearchTree* new_to_right= to->right;
    BinarySearchTree**new_to_link = tree;
    if(from->parent)
        new_to_link = from->parent->left == from ? &from->parent->left : &from->parent->right;

    if(to->parent == from){
        new_from_parent = to;
        new_from_parent = NULL;
        if(from->left==to)
            new_to_left = from;
        else
            new_to_right=from;
    }
    else if (from->parent == to){
        new_to_parent = from;
        new_to_link = NULL;
        if(to->left == from)
            new_from_left = to;
        else
            new_from_right = to;
    }
    from->parent = new_from_parent;
    from->left = new_from_left;
    if(from->left)
        from->left->parent = from;
    from->right = new_from_right;
    if(from->right)
        from->right->parent = from;
    if(new_from_link)
        *new_from_link = from;

    to->parent = new_to_parent;
    to->left = new_to_left;
    if(to->left)
        to->left->parent = to;
    to->right = new_to_right;
    if(to->right)
        to->right->parent = to;
    if(new_to_link)
        *new_to_link = to;

}


// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));

    ptrBinarySearchTree predecessor = (ptrBinarySearchTree) bstree_predecessor(current);
    bstree_swap_nodes(t,current,predecessor);
    if(current) bstree_delete(&current);
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree current = (ptrBinarySearchTree)bstree_search(*t,v);

    if(current) bstree_remove_node(t,current);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t))
        return;
    
    f(t,environment);
    bstree_depth_prefix(bstree_left(t),f,environment);
    bstree_depth_prefix(bstree_right(t),f,environment);
    return;
}

//on remarque que cela affiche les valeurs dans l'ordre
void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t))
        return;
    
    bstree_depth_infix(bstree_left(t),f,environment);
    f(t,environment);
    bstree_depth_infix(bstree_right(t),f,environment);
    return;
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t))
        return;

    bstree_depth_postfix(t->left,f,environment);
    bstree_depth_postfix(bstree_right(t),f,environment);
    f(t,environment);
    return;

}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    Queue* waiting_q = create_queue();
    queue_push(waiting_q,t);
    
    while (!queue_empty(waiting_q)){
        f(queue_top(waiting_q),environment);
        if (!bstree_empty(bstree_left(queue_top(waiting_q))))
            queue_push(waiting_q,bstree_left(queue_top(waiting_q)));
        if (!bstree_empty(bstree_right(queue_top(waiting_q))))
            queue_push(waiting_q,bstree_right(queue_top(waiting_q)));
        queue_pop(waiting_q);
        
    }
    
    delete_queue(&waiting_q);
}

void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    printf("\nbegining infix\n");
    while (!bstree_predecessor(t))
        t = bstree_predecessor(t);

    while (!bstree_successor(t)){
        f(t,environment);
        t = bstree_successor(t);
    }
    printf("\nend infix\n");
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	(void)collection; (void)direction;
	return NULL;
}

/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

