/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:	Provides a skip list abstract data type.
 *
 *              (See "Skip Lists: A Probabilistic Alternative to Balanced Trees"
 *               by William Pugh for additional information)
 *
 *              (This implementation has the optimization for reducing key
 *               key comparisons mentioned in section 3.5 of "A Skip List
 *               Cookbook" by William Pugh)
 *
 *              (Also, this implementation has a couple of home-grown
 *               optimizations, including setting the "update" vector to the
 *               actual 'forward' pointer to update, instead of the node
 *               containing the forward pointer -QAK)
 *
 */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5SL_init_interface

/* Pablo information */
/* (Put before include files to avoid problems with inline functions) */
#define PABLO_MASK	H5SL_mask

/* Private headers needed */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5FLprivate.h"	/* Free Lists                           */
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5SLprivate.h"	/* Skip list routines			*/

/* Local Macros */

/* Define the code template for insertions for the "OP" in the H5SL_FIND macro */
#define H5SL_FIND_INSERT_FOUND(SLIST,X,UPDATE,I,ITEM)                   \
        HGOTO_DONE(FAIL);

/* Define the code template for removals for the "OP" in the H5SL_FIND macro */
#define H5SL_FIND_REMOVE_FOUND(SLIST,X,UPDATE,I,ITEM)                   \
        void *tmp;                                                      \
                                                                        \
        for(I=0; I<=(int)SLIST->curr_level; I++) {                      \
            if(*UPDATE[I]!=X)                                           \
                break;                                                  \
            *UPDATE[I]=X->forward[I];                                   \
        } /* end for */                                                 \
        tmp=X->item;                                                    \
        H5MM_xfree(X);                                                  \
        while(SLIST->curr_level>0 && SLIST->header->forward[SLIST->curr_level]==NULL) \
            SLIST->curr_level--;                                        \
        SLIST->nobjs--;                                                 \
        HGOTO_DONE(tmp);

/* Define the code template for searches for the "OP" in the H5SL_FIND macro */
#define H5SL_FIND_SEARCH_FOUND(SLIST,X,UPDATE,I,ITEM)                   \
        HGOTO_DONE(X->item);

/* Define a code template for updating the "update" vector for the "DOUPDATE" in the H5SL_FIND macro */
#define H5SL_FIND_YES_UPDATE(X,UPDATE,I)                                \
        UPDATE[I]=&X->forward[I];

/* Define a code template for _NOT_ updating the "update" vector for the "DOUPDATE" in the H5SL_FIND macro */
#define H5SL_FIND_NO_UPDATE(X,UPDATE,I)

/* Define a code template for comparing scalar keys for the "CMP" in the H5SL_FIND macro */
#define H5SL_FIND_SCALAR_CMP(TYPE,PKEY1,PKEY2)                          \
        (*(TYPE *)PKEY1<*(TYPE *)PKEY2)

/* Define a code template for comparing string keys for the "CMP" in the H5SL_FIND macro */
#define H5SL_FIND_STRING_CMP(TYPE,PKEY1,PKEY2)                          \
        (HDstrcmp(*(TYPE *)PKEY1,*(TYPE *)PKEY2)<0)

/* Define a code template for comparing scalar keys for the "EQ" in the H5SL_FIND macro */
#define H5SL_FIND_SCALAR_EQ(TYPE,PKEY1,PKEY2)                           \
        (*(TYPE *)PKEY1==*(TYPE *)PKEY2)

/* Define a code template for comparing string keys for the "EQ" in the H5SL_FIND macro */
#define H5SL_FIND_STRING_EQ(TYPE,PKEY1,PKEY2)                           \
        (HDstrcmp(*(TYPE *)PKEY1,*(TYPE *)PKEY2)==0)

/* Macro used to find node for operation */
#define H5SL_FIND(OP,DOUPDATE,CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED) \
    CHECKED=NULL;                                                       \
    for(I=(int)SLIST->curr_level; I>=0; I--) {                          \
        if(X->forward[I]!=CHECKED) {                                    \
            while(X->forward[I] && H5_GLUE3(H5SL_FIND_,CMP,_CMP)(TYPE,X->forward[I]->key,KEY) ) \
                X=X->forward[I];                                        \
            CHECKED=X->forward[I];                                      \
        } /* end if */                                                  \
        H5_GLUE3(H5SL_FIND_,DOUPDATE,_UPDATE)(X,UPDATE,I)               \
    } /* end for */                                                     \
    X=X->forward[0];                                                    \
    if(X!=NULL && H5_GLUE3(H5SL_FIND_,CMP,_EQ)(TYPE,X->key,KEY) ) {     \
        /* What to do when a node is found */				\
        H5_GLUE3(H5SL_FIND_,OP,_FOUND)(SLIST,X,UPDATE,I,ITEM)           \
    } /* end if */

/* Macro used to insert node */
#define H5SL_INSERT(CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)         \
    H5SL_FIND(INSERT,YES,CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)

/* Macro used to remove node */
#define H5SL_REMOVE(CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)         \
    H5SL_FIND(REMOVE,YES,CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)

/* Macro used to search for node */
#define H5SL_SEARCH(CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)         \
    H5SL_FIND(SEARCH,NO,CMP,SLIST,X,UPDATE,I,TYPE,ITEM,KEY,CHECKED)


/* Private typedefs & structs */

/* Skip list node data structure */
struct H5SL_node_t {
    void *key;                          /* Pointer to node's key */
    void *item;                         /* Pointer to node's item */
    size_t level;                       /* The level of this node */
    struct H5SL_node_t **forward;       /* Array of forward pointers from this node */
};

/* Main skip list data structure */
struct H5SL_t {
    /* Static values for each list */
    H5SL_type_t type;   /* Type of skip list */
    double p;           /* Probability of using a higher level [0..1) */
    int p1;             /* Probability converted into appropriate value for random # generator on this machine */
    size_t max_level;   /* Maximum number of levels */

    /* Dynamic values for each list */
    int curr_level;             /* Current top level used in list */
    size_t nobjs;               /* Number of active objects in skip list */
    H5SL_node_t *header;        /* Header for nodes in skip list */
};

/* Static functions */
static size_t H5SL_random_level(int p1, size_t max_level);
static H5SL_node_t * H5SL_new_node(size_t lvl, void *item, void *key);

/* Declare a free list to manage the H5SL_t struct */
H5FL_DEFINE_STATIC(H5SL_t);


/*--------------------------------------------------------------------------
 NAME
    H5SL_init_interface
 PURPOSE
    Initialize interface-specific information
 USAGE
    herr_t H5SL_init_interface()

 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    Initializes any interface-specific data or routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static herr_t
H5SL_init_interface(void)
{
    time_t curr_time;   /* Current time, for seeding random number generator */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_init_interface)

    /* Create randomized set of numbers */
    curr_time=HDtime(NULL);
    HDsrand((unsigned)curr_time);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5SL_init_interface() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_random_level
 PURPOSE
    Generate a random level
 USAGE
    size_t H5SL_random_level(p,max_level)
        int p1;                 IN: probability distribution
        size_t max_level;       IN: Maximum level for node height

 RETURNS
    Returns non-negative level value
 DESCRIPTION
    Count elements in a skip list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Do we really need a 'random' value, or is a series of nodes with the
    correct heights "good enough".  We could track the state of the nodes
    allocated for this list and issue node heights appropriately (i.e. 1,2,
    1,4,1,2,1,8,...) (or would that be 1,1,2,1,1,2,4,... ?)
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static size_t
H5SL_random_level(int p1, size_t max_level)
{
    size_t lvl;         /* Level generated */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_random_level);

    /* Account for starting at zero offset */
    max_level--;

    lvl=0;
    while(HDrand()<p1 && lvl < max_level)
        lvl++;

    FUNC_LEAVE_NOAPI(lvl);
} /* end H5SL_random_level() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_new_node
 PURPOSE
    Create a new skip list node
 USAGE
    H5SL_node_t *H5SL_new_node(lvl,item,key)
        size_t lvl;             IN: Level for node height
        void *item;             IN: Pointer to item info for node
        void *key;              IN: Pointer to key info for node

 RETURNS
    Returns a pointer to a skip list node on success, NULL on failure.
 DESCRIPTION
    Create a new skip list node of the specified height, setting the item
    and key values internally.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    This routine does _not_ initialize the 'forward' pointers

    We should set up a custom free-list for allocating & freeing these sort
    of nodes.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static H5SL_node_t *
H5SL_new_node(size_t lvl, void *item, void *key)
{
    H5SL_node_t *ret_value;      /* New skip list node */

    FUNC_ENTER_NOAPI_NOINIT(H5SL_new_node);

    /* Allocate the node */
    if((ret_value=H5MM_malloc(sizeof(H5SL_node_t)+(sizeof(H5SL_node_t *)*(lvl+1))))==NULL)
        HGOTO_ERROR(H5E_SLIST,H5E_NOSPACE,NULL,"memory allocation failed");

    /* Initialize node */
    ret_value->key=key;
    ret_value->item=item;
    ret_value->level=lvl;
    ret_value->forward=(H5SL_node_t **)((unsigned char *)ret_value+sizeof(H5SL_node_t));

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_new_node() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_create
 PURPOSE
    Create a skip list
 USAGE
    H5SL_t *H5SL_create(void)

 RETURNS
    Returns a pointer to a skip list on success, NULL on failure.
 DESCRIPTION
    Create a skip list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
H5SL_t *
H5SL_create(H5SL_type_t type, double p, size_t max_level)
{
    H5SL_t *new_slist=NULL;     /* Pointer to new skip list object created */
    H5SL_node_t *header;        /* Pointer to skip list header node */
    size_t u;                   /* Local index variable */
    H5SL_t *ret_value;          /* Return value */

    FUNC_ENTER_NOAPI(H5SL_create,NULL);

    /* Check args */
    HDassert(p>0.0 && p<1.0);
    HDassert(max_level>0 && max_level<=H5SL_LEVEL_MAX);
    HDassert(type>=H5SL_TYPE_INT && type<=H5SL_TYPE_STR);

    /* Allocate skip list structure */
    if((new_slist=H5FL_MALLOC(H5SL_t))==NULL)
        HGOTO_ERROR(H5E_SLIST,H5E_NOSPACE,NULL,"memory allocation failed");

    /* Set the static internal fields */
    new_slist->type=type;
    new_slist->p=p;
    new_slist->p1=p*RAND_MAX;
    new_slist->max_level=max_level;

    /* Set the dynamic internal fields */
    new_slist->curr_level=-1;
    new_slist->nobjs=0;

    /* Allocate the header node */
    if((header=H5SL_new_node(max_level-1,NULL,NULL))==NULL)
        HGOTO_ERROR(H5E_SLIST,H5E_NOSPACE,NULL,"memory allocation failed");

    /* Initialize header node's forward pointers */
    for(u=0; u<max_level; u++)
        header->forward[u]=NULL;

    /* Attach the header */
    new_slist->header=header;

    /* Set the return value */
    ret_value=new_slist;

done:
    /* Error cleanup */
    if(ret_value==NULL) {
        if(new_slist!=NULL) {
            H5FL_FREE(H5SL_t,new_slist);
        } /* end if */
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_create() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_count
 PURPOSE
    Count the number of objects in a skip list
 USAGE
    ssize_t H5SL_count(slist)
        H5SL_t *slist;            IN: Pointer to skip list to count

 RETURNS
    Returns non-negative on success, negative on failure.
 DESCRIPTION
    Count elements in a skip list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
ssize_t
H5SL_count(H5SL_t *slist)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_count);

    /* Check args */
    assert(slist);

    /* Check internal consistency */
    /* (Pre-condition) */

    FUNC_LEAVE_NOAPI(slist->nobjs);
} /* end H5SL_count() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_insert
 PURPOSE
    Insert an objects into a skip list
 USAGE
    herr_t H5SL_insert(slist,item,key)
        H5SL_t *slist;          IN/OUT: Pointer to skip list
        void *item;             IN: Item to insert
        void *key;              IN: Key for item to insert

 RETURNS
    Returns non-negative on success, negative on failure.
 DESCRIPTION
    Insert element into a skip list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Inserting an item with the same key as an existing object fails.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t
H5SL_insert(H5SL_t *slist, void *item, void *key)
{
    H5SL_node_t **update[H5SL_LEVEL_MAX];       /* 'update' vector */
    H5SL_node_t *checked;                       /* Pointer to last node checked */
    H5SL_node_t *x;                             /* Current node to examine */
    size_t lvl;                                 /* Level of new node */
    int i;                                      /* Local index value */
    herr_t ret_value=SUCCEED;                   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5SL_insert);

    /* Check args */
    assert(slist);
    assert(key);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Insert item into skip list */

    /* Work through the forward pointers for a node, finding the node at each
     * level that is before the location to insert
     */
    x=slist->header;
    switch(slist->type) {
        case H5SL_TYPE_INT:
            H5SL_INSERT(SCALAR,slist,x,update,i,int,item,key,checked)
            break;

        case H5SL_TYPE_HADDR:
            H5SL_INSERT(SCALAR,slist,x,update,i,haddr_t,item,key,checked)
            break;

        case H5SL_TYPE_STR:
            H5SL_INSERT(STRING,slist,x,update,i,char *,item,key,checked)
            break;
    } /* end switch */

    /* 'key' must not have been found in existing list, if we get here */

    /* Generate level for new node */
    lvl=H5SL_random_level(slist->p1,slist->max_level);
    if((int)lvl>slist->curr_level) {
        /* Cap the increase in the current level to just one greater */
        lvl=slist->curr_level+1;

        /* Set the update pointer correctly */
        update[lvl]=&slist->header->forward[lvl];

        /* Increase the maximum level of the list */
        slist->curr_level=(int)lvl;
    } /* end if */

    /* Create new node of proper level */
    if((x=H5SL_new_node(lvl,item,key))==NULL)
        HGOTO_ERROR(H5E_SLIST,H5E_NOSPACE,FAIL,"can't create new skip list node");

    /* Link the new node into the existing list */
    for(i=0; i<=(int)lvl; i++) {
        x->forward[i]=*update[i];
        *update[i]=x;
    } /* end for */

    /* Increment the number of nodes in the skip list */
    slist->nobjs++;

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_insert() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_search
 PURPOSE
    Search for objects into a skip list
 USAGE
    void *H5SL_search(slist,key)
        H5SL_t *slist;          IN/OUT: Pointer to skip list
        void *key;              IN: Key for item to search for

 RETURNS
    Returns pointer to item on success, NULL on failure
 DESCRIPTION
    Search for an object in a skip list, according to it's key
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void *
H5SL_search(H5SL_t *slist, void *key)
{
    H5SL_node_t *checked;                       /* Pointer to last node checked */
    H5SL_node_t *x;                             /* Current node to examine */
    int i;                                      /* Local index value */
    void *ret_value;                            /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_search);

    /* Check args */
    assert(slist);
    assert(key);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Insert item into skip list */

    /* Work through the forward pointers for a node, finding the node at each
     * level that is before the location to insert
     */
    x=slist->header;
    switch(slist->type) {
        case H5SL_TYPE_INT:
            H5SL_SEARCH(SCALAR,slist,x,-,i,int,-,key,checked)
            break;

        case H5SL_TYPE_HADDR:
            H5SL_SEARCH(SCALAR,slist,x,-,i,haddr_t,-,key,checked)
            break;

        case H5SL_TYPE_STR:
            H5SL_SEARCH(STRING,slist,x,-,i,char *,-,key,checked)
            break;
    } /* end switch */

    /* 'key' must not have been found in existing list, if we get here */
    ret_value=NULL;

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_search() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_remove
 PURPOSE
    Removes an object from a skip list
 USAGE
    void *H5SL_remove(slist,key)
        H5SL_t *slist;          IN/OUT: Pointer to skip list
        void *key;              IN: Key for item to remove

 RETURNS
    Returns pointer to item removed on success, NULL on failure.
 DESCRIPTION
    Remove element from a skip list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void *
H5SL_remove(H5SL_t *slist, void *key)
{
    H5SL_node_t **update[H5SL_LEVEL_MAX];       /* 'update' vector */
    H5SL_node_t *checked;                       /* Pointer to last node checked */
    H5SL_node_t *x;                             /* Current node to examine */
    int i;                                      /* Local index value */
    void *ret_value=NULL;                       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_remove);

    /* Check args */
    assert(slist);
    assert(key);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Remove item from skip list */

    /* Work through the forward pointers for a node, finding the node at each
     * level that is before the location to remove
     */
    x=slist->header;
    switch(slist->type) {
        case H5SL_TYPE_INT:
            H5SL_REMOVE(SCALAR,slist,x,update,i,int,-,key,checked)
            break;

        case H5SL_TYPE_HADDR:
            H5SL_REMOVE(SCALAR,slist,x,update,i,haddr_t,-,key,checked)
            break;

        case H5SL_TYPE_STR:
            H5SL_REMOVE(STRING,slist,x,update,i,char *,-,key,checked)
            break;
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_remove() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_first
 PURPOSE
    Gets a pointer to the first node in a skip list
 USAGE
    H5SL_node_t *H5SL_first(slist)
        H5SL_t *slist;          IN: Pointer to skip list

 RETURNS
    Returns pointer to first node in skip list on success, NULL on failure.
 DESCRIPTION
    Retrieves a pointer to the first node in a skip list, for iterating over
    the list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
H5SL_node_t *
H5SL_first(H5SL_t *slist)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_first);

    /* Check args */
    assert(slist);

    /* Check internal consistency */
    /* (Pre-condition) */

    FUNC_LEAVE_NOAPI(slist->header->forward[0]);
} /* end H5SL_first() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_next
 PURPOSE
    Gets a pointer to the next node in a skip list
 USAGE
    H5SL_node_t *H5SL_next(slist_node)
        H5SL_node_t *slist_node;          IN: Pointer to skip list node

 RETURNS
    Returns pointer to node after slist_node in skip list on success, NULL on failure.
 DESCRIPTION
    Retrieves a pointer to the next node in a skip list, for iterating over
    the list.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
H5SL_node_t *
H5SL_next(H5SL_node_t *slist_node)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_next);

    /* Check args */
    assert(slist_node);

    /* Check internal consistency */
    /* (Pre-condition) */

    FUNC_LEAVE_NOAPI(slist_node->forward[0]);
} /* end H5SL_next() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_item
 PURPOSE
    Gets pointer to the 'item' for a skip list node
 USAGE
    void *H5SL_item(slist_node)
        H5SL_node_t *slist_node;          IN: Pointer to skip list node

 RETURNS
    Returns pointer to node 'item' on success, NULL on failure.
 DESCRIPTION
    Retrieves a node's 'item'
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void *
H5SL_item(H5SL_node_t *slist_node)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_item);

    /* Check args */
    assert(slist_node);

    /* Check internal consistency */
    /* (Pre-condition) */

    FUNC_LEAVE_NOAPI(slist_node->item);
} /* end H5SL_item() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_iterate
 PURPOSE
    Iterate over all nodes in a skip list
 USAGE
    herr_t H5SL_iterate(slist, op, op_data)
        H5SL_t *slist;          IN/OUT: Pointer to skip list to iterate over
        H5SL_operator_t op;     IN: Callback function for iteration
        void *op_data;          IN/OUT: Pointer to application data for callback

 RETURNS
    Returns a negative value if something is wrong, the return
        value of the last operator if it was non-zero, or zero if all
        nodes were processed.
 DESCRIPTION
    Iterate over all the nodes in a skip list, calling an application callback
    with the item, key and any operator data.

    The operator callback receives a pointer to the item and key for the list
    being iterated over ('mesg'), and the pointer to the operator data passed
    in to H5SL_iterate ('op_data').  The return values from an operator are:
        A. Zero causes the iterator to continue, returning zero when all 
            nodes of that type have been processed.
        B. Positive causes the iterator to immediately return that positive
            value, indicating short-circuit success.
        C. Negative causes the iterator to immediately return that value,
            indicating failure.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t
H5SL_iterate(H5SL_t *slist, H5SL_operator_t op, void *op_data)
{
    H5SL_node_t *node;  /* Pointers to skip list nodes */
    herr_t ret_value=0; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_iterate);

    /* Check args */
    assert(slist);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Free skip list nodes */
    node=slist->header->forward[0];
    while(node!=NULL) {
        /* Call the iterator callback */
        if((ret_value=(op)(node->item,node->key,op_data))!=0)
            break;

        node=node->forward[0];
    } /* end while */

    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5SL_iterate() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_release
 PURPOSE
    Release all nodes from a skip list
 USAGE
    herr_t H5SL_release(slist)
        H5SL_t *slist;            IN/OUT: Pointer to skip list to release nodes

 RETURNS
    Returns non-negative on success, negative on failure.
 DESCRIPTION
    Release all the nodes in a skip list.  Any objects left in the skip list
    nodes are not deallocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t
H5SL_release(H5SL_t *slist)
{
    H5SL_node_t *node, *next_node;      /* Pointers to skip list nodes */
    size_t u;                   /* Local index variable */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_release);

    /* Check args */
    assert(slist);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Free skip list nodes */
    node=slist->header->forward[0];
    while(node!=NULL) {
        next_node=node->forward[0];
        H5MM_xfree(node);
        node=next_node;
    } /* end while */

    /* Reset the header pointers */
    for(u=0; u<slist->max_level; u++)
        slist->header->forward[u]=NULL;

    /* Reset the dynamic internal fields */
    slist->curr_level=-1;
    slist->nobjs=0;

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5SL_release() */


/*--------------------------------------------------------------------------
 NAME
    H5SL_close
 PURPOSE
    Close a skip list, deallocating it.
 USAGE
    herr_t H5SL_close(slist)
        H5SL_t *slist;            IN/OUT: Pointer to skip list to close

 RETURNS
    Returns non-negative on success, negative on failure.
 DESCRIPTION
    Close a skip list, freeing all internal information.  Any objects left in
    the skip list are not deallocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t
H5SL_close(H5SL_t *slist)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5SL_close);

    /* Check args */
    assert(slist);

    /* Check internal consistency */
    /* (Pre-condition) */

    /* Free skip list nodes */
    H5SL_release(slist);        /* always succeeds */

    /* Release header node */
    H5MM_xfree(slist->header);

    /* Free skip list object */
    H5FL_FREE(H5SL_t,slist);

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5SL_close() */

