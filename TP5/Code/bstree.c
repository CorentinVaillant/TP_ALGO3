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
    ((BinarySearchTree*) t)->key=0xF0431D;
    ((BinarySearchTree*) t)->left=NULL;
    ((BinarySearchTree*) t)->right=NULL;
    if(t->parent && t->parent->left == t) 
         ((BinarySearchTree*) t)->parent->left =NULL;
    if(t->parent && t->parent->right== t)
         ((BinarySearchTree*) t)->parent->right=NULL;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);

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

void bstree_swap_nodes(ptrBinarySearchTree *tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));

    BinarySearchTree *temp = from->parent;
    from->parent = to->parent;
    to->parent = temp;

    if (from->parent) {
        if (from->parent->left == to) from->parent->left = from;
        else from->parent->right = from;
    } else {
        *tree = from;
    }

    if (to->parent) {
        if (to->parent->left == from) to->parent->left = to;
        else to->parent->right = to;
    } else {
        *tree = to;
    }

    BinarySearchTree *tmpLeft = from->left;
    BinarySearchTree *tmpRight = from->right;
    from->left = to->left;
    from->right = to->right;
    to->left = tmpLeft;
    to->right = tmpRight;

    if (from->left) from->left->parent = from;
    if (from->right) from->right->parent = from;
    if (to->left) to->left->parent = to;
    if (to->right) to->right->parent = to;
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree *t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));

    ptrBinarySearchTree d;
    if (!current->left && !current->right) d = NULL;
    else if (!current->left) d = current->right;
    else if (!current->right) d = current->left;
    else {
        ptrBinarySearchTree succ = (ptrBinarySearchTree)bstree_successor(current);
        bstree_swap_nodes(t, current, succ);
        d = current->right;
    }

    if (d != NULL) {
        d->parent = current->parent;
    }

    if (!current->parent) *t = d;
    else if (current->parent->left == current) current->parent->left = d;
    else current->parent->right = d;

    free(current);
}
void bstree_remove(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree current = (ptrBinarySearchTree)bstree_search(*t,v);

    if(!bstree_empty(current) && !bstree_empty(*t)){
        bstree_remove_node(t,current);
    }
    //else the node does not exist
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
    if(bstree_empty(t)) return;
    while (!bstree_empty(bstree_predecessor(t)))
        t = bstree_predecessor(t);

    if(bstree_empty(t)) return;
    while (!bstree_empty(bstree_successor(t))){
        f(t,environment);
        t = bstree_successor(t);
    }
    f(t,environment);
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
	while (e->left)
        e = e->left;
    return e;
    
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	while (e->right)
        e = e->right;
    return e;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	BSTreeIterator * iterator = malloc(sizeof(BSTreeIterator));
    if(!iterator){
        fprintf(stderr, "error while allocate the memory for iterator\n");
        perror("stopping the proram\n");
        exit(1);
    }
    
    iterator->collection = collection;
    iterator->begin = direction == forward 
        ? goto_min
        : goto_max;

    iterator->next = direction == forward
        ? bstree_successor
        : bstree_predecessor;
    return iterator;

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

