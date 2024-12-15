#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/
typedef enum {red, black} NodeColor;

struct _bstree {
    NodeColor color;

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
#define nonNull(ptr) ptr ;assert(ptr != NULL);



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
    t->color = red;
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

BinarySearchTree *fixredblack_insert(BinarySearchTree *x);

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
	
    ptrBinarySearchTree *curr = t;
    ptrBinarySearchTree par = NULL;
    while (!bstree_empty(*curr)){
        par = *curr;
        if((*curr)->key > v)
            curr = &((*curr)-> left);
        else 
            curr = &((*curr)->right);
    }
    *curr = bstree_cons(NULL,NULL,v);
    (*curr)->parent = par;


    BinarySearchTree* top = fixredblack_insert(*curr);
    if(top->parent == NULL)
        *t = top;
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


ptrBinarySearchTree fixredblack_remove(ptrBinarySearchTree p, ptrBinarySearchTree x);

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree *t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));

    ptrBinarySearchTree m;
    if (!current->left && !current->right) m = NULL;
    else if (!current->left) m = current->right;
    else if (!current->right) m = current->left;
    else {
        ptrBinarySearchTree succ = (ptrBinarySearchTree)bstree_successor(current);
        bstree_swap_nodes(t, current, succ);
        m = current->right;
    }

    if (m != NULL) m->parent = current->parent;
    

    if (!current->parent) *t = m;
    else if (current->parent->left == current) current->parent->left = m;
    else current->parent->right = m;

/*------fixing the tree------*/

    ptrBinarySearchTree substitute = m;

    /* fix the redblack properties if needed */
    if (color(current) == black){
        if ((substitute == NULL) || (substitute->color == black)){

            /* substitute is double black : must fix */
            ptrBinarySearchTree subtreeroot = fixredblack_remove (current->parent,substitute);

            if ( subtreeroot->parent == NULL )
                *t = subtreeroot ;
        }else 
            /* substitute becomes black */
            substitute->color = black ;
    }

    /* free the memory */
    free(current);

}
void bstree_remove(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree current = (ptrBinarySearchTree)bstree_search(*t,v);

    if(!bstree_empty(current) && !bstree_empty(*t)){
        bstree_remove_node(t,current);
    }
    //else the node does not exist
}


/*--------------------  RedBlackTree Operators  ---------------------*/

NodeColor color(const BinarySearchTree *x){
    return x && x->color == red
        ? red
        : black;
}

void leftrotate(BinarySearchTree *x){
    assert(!bstree_empty(x)&&!bstree_empty(x->right));

    ptrBinarySearchTree x_parent_or_x = x->parent ? x->parent : x;
    ptrBinarySearchTree y = x->right;


    //swapping x and left child : y
    bstree_swap_nodes(&x_parent_or_x,x,y);

    //swaping the two children of y.
    //* x = y->right
    y->right = y->left;
    y->left = x;

    //swapping the children of x
    ptrBinarySearchTree tmp_left_x = x->left;
    x->left = x->right;
    x->right= tmp_left_x;

    //swapping left child of x with right child of y
    ptrBinarySearchTree tmp_right_y = y->right;
    y->right= x->left;
    x->left = tmp_right_y;

    if(y->right) y->right->parent = y;
    if(x->left)  x->left->parent = x;

}
void rightrotate(BinarySearchTree *y){
    assert(!bstree_empty(y)&&!bstree_empty(y->left));

    ptrBinarySearchTree y_parent_or_y = y->parent ? y->parent : y;
    ptrBinarySearchTree x = y->left;

    //swapping y and right child : x
    bstree_swap_nodes(&y_parent_or_y,y,x);

    //swapping the children of y
    //* y = x->right
    x->left = x->right;
    x->right = y;

    //swapping the children of y
    ptrBinarySearchTree tmp_right_y = y->right; 
    y->right= y->left;
    y->left = tmp_right_y;

    //swapping right child of y with left child of x
    ptrBinarySearchTree tmp_left_x = x->left;
    x->left = y->right;
    y->right= tmp_left_x;

    if(x->left)  x->left->parent = x;
    if(y->right) y->right->parent = y;
}

BinarySearchTree* grandparent(BinarySearchTree* n){
    return n->parent 
        ? n->parent->parent
        : NULL;
}
BinarySearchTree* uncle(BinarySearchTree* n){
    return grandparent(n)
        ?grandparent(n)->left == n->parent 
            ?grandparent(n)->right
            :grandparent(n)->left
        :NULL;
}
BinarySearchTree* sibling(BinarySearchTree* n){
    return n->parent
        ? n == n->parent->left 
            ? n->parent->right
            : n->parent->left
        : NULL;
}


BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x);

//Léo, sache que si tu touche a ce code, j'aurais moi même le droit de te toucher très fort
//Cela m'embête un peu que tu tombe vite dans la facilité de piquer mon code, je suis dans le même cas que toi, j'y passe beaucoup trop de temps aussi
//Essaie au moins de refacto des trucs pour pas que cela soit cramer stp.
BinarySearchTree* fixredblack_insert(BinarySearchTree* x){
    ptrBinarySearchTree pp = grandparent(x);
    if(bstree_empty(pp)){
        if(x->parent)
            x->parent->color = black;
        else
            x->color = black;
        
        return x;
    }

    if(color(x->parent) == red || color(x->left) == red || color(x->right) == red)
        return fixredblack_insert_case1(x);
    else return x;


}

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x){

    ptrBinarySearchTree f = uncle(x);
    ptrBinarySearchTree p = nonNull(x->parent);
    ptrBinarySearchTree pp= nonNull(grandparent(x));

    //not in the case1, handling case 2
    if(color(f) == black)
        return fixredblack_insert_case2(x);
    
    f->color = black;
    p->color= black;
    pp->color=red;

    //fixing the grandparent 
    return fixredblack_insert(pp);    
}
BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x){

    ptrBinarySearchTree p = nonNull(x->parent);
    ptrBinarySearchTree pp= nonNull(grandparent(x));


    if(p == pp->left)
        return fixredblack_insert_case2_left(x);
    else
        return fixredblack_insert_case2_right(x);
    
}
BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x){
    ptrBinarySearchTree p = nonNull(x->parent);
    ptrBinarySearchTree pp= nonNull(grandparent(x)); //?

    if(p->left == x){
        rightrotate(pp);
        p->color = black;
        pp->color = red;
        
    }
    else{
        leftrotate(p);

        rightrotate(pp);
        x->color = black;
        pp->color = red;

    }
    return x;
}
BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x){
    ptrBinarySearchTree p = nonNull(x->parent);
    ptrBinarySearchTree pp= nonNull(grandparent(x));

    if(p->right == x){
        leftrotate(pp);
        p->color = black;
        pp->color = red;
    }
    else{
        rightrotate(p);


        leftrotate(pp);
        x->color = black;
        pp->color = red;
    }
    return x;
    
}


BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p);

BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p);

ptrBinarySearchTree fixredblack_remove(ptrBinarySearchTree p, ptrBinarySearchTree x){
    if(!x || !x->parent)
        return x;
    if(color(sibling(x)) == black)
        return fixredblack_remove_case1(p,x);
    else 
        return fixredblack_remove_case2(p);
}

BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x){
    if(p->left == x)
        return fixredblack_remove_case1_left(p);
    else
        return fixredblack_remove_case1_right(p);
}
BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p){
    ptrBinarySearchTree f = nonNull(p->right);
    ptrBinarySearchTree d = f->right;
    ptrBinarySearchTree g = f->left;

    if(color(g) == black && color(d) == black){ //? not sure that f is not NULL ? 
        f->color = red;
        fixredblack_remove(p->parent,p); //...
    }
    else if(color(d) == red){
        leftrotate(p);
        f->color = p->color;
        p->color = black;
        f->color = black;
        return p;        
    }
    else{
        rightrotate(f);
        g->color = black;
        f->color = red;
        leftrotate(p);
        f->color = p->color;
        return p;
    }
}
BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p){
    ptrBinarySearchTree f = nonNull(p->right);
    ptrBinarySearchTree d = f->right;
    ptrBinarySearchTree g = f->left;

    if(color(g) == black && color(d) == black){ //? not sure that f is not NULL ? 
        f->color = red;
        fixredblack_remove(p->parent,p); //...
    }
    else if(color(g) == red){
        rightrotate(p);
        f->color = p->color;
        p->color = black;
        f->color = black;
        return p;        
    }
    else{
        leftrotate(f);
        d->color = black;
        f->color = red;
        rightrotate(p);
        f->color = p->color;
        return p;
    }
}

BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x){
    if(p->left == x)
        return fixredblack_remove_case2_left(p);
    else
        return fixredblack_remove_case2_right(p);
}
BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p){
    ptrBinarySearchTree f = p->right; //red
    leftrotate(p);
    p->color = red;
    f->color = black;
    fixredblack_remove(p,p->left);
}
BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p){
    ptrBinarySearchTree f = p->left; //red
    rightrotate(p);
    p->color = red;
    f->color = black;
    fixredblack_remove(p,p->right);
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


/*-----------------------  RedBlackTree Test  -----------------------*/


void testrotateleft (BinarySearchTree* t) {
    leftrotate (t) ;
}
void testrotateright (BinarySearchTree* t) {
    rightrotate (t);
}



/*--------------------------  Interface  ---------------------------*/

void bstree_node_to_dot(const BinarySearchTree* t, void* stream){
    FILE *file = (FILE *) stream;

    printf("%d ", bstree_key(t));
    fprintf(file, "\tn%d [label=\"{%d|{<left>|<right>}}\",style=filled, fillcolor=%s];\n",
            bstree_key(t), bstree_key(t), 
            t->color == red ? "\"red\"" : "\"gray\"");

    if (bstree_left(t)) {
        fprintf(file, "\tn%d:left:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_left(t)));
    } else {
        fprintf(file, "\tlnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:left:c -> lnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
    if (bstree_right(t)) {
        fprintf(file, "\tn%d:right:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_right(t)));
    } else {
        fprintf(file, "\trnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:right:c -> rnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
}