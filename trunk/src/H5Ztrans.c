/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "H5Zprivate.h"

typedef struct result {
    H5Z_token_type type;
    H5Z_num_val    value;
    H5T_class_t ar_type;
} H5Z_result;


/* The token */
typedef struct {
    const char *tok_expr;       /* Holds the original expression        */

    /* Current token values */
    H5Z_token_type  tok_type;       /* The type of the current token        */
    const char *tok_begin;      /* The beginning of the current token   */
    const char *tok_end;        /* The end of the current token         */

    /* Previous token values */
    H5Z_token_type  tok_last_type;  /* The type of the last token           */
    const char *tok_last_begin; /* The beginning of the last token      */
    const char *tok_last_end;   /* The end of the last token            */
} H5Z_token;




/* Interface initialization */
static int interface_initialize_g = 0;
#define INTERFACE_INIT NULL 

static H5Z_token *H5Z_get_token(H5Z_token *current);
static H5Z_node *H5Z_parse_expression(H5Z_token *current);
static H5Z_node *H5Z_parse_term(H5Z_token *current);
static H5Z_node *H5Z_parse_factor(H5Z_token *current);
static H5Z_node *H5Z_new_node(H5Z_token_type type);
static void H5Z_do_op(H5Z_node* tree);
static H5Z_result H5Z_eval_full(H5Z_node *tree, void* array, hsize_t array_size, hid_t array_type);


#ifdef H5Z_XFORM_DEBUG
static void H5Z_XFORM_DEBUG(H5Z_node *tree);
static void H5Z_print(H5Z_node *tree, FILE *stream);
#endif  /* H5Z_XFORM_DEBUG */


/*
 *  Programmer: Bill Wendling <wendling@ncsa.uiuc.edu>
 *              25. August 2003
 */

/*
 * This is the context-free grammar for our expressions:
 *
 * expr     :=  term    | term '+ term      | term '-' term
 * term     :=  factor  | factor '*' factor | factor '/' factor
 * factor   :=  number      |
 *              symbol      |
 *              '-' factor  |   // unary minus
 *              '+' factor  |   // unary plus
 *              '(' expr ')'
 * symbol   :=  [a-zA-Z][a-zA-Z0-9]*
 * number   :=  H5Z_INTEGER | FLOAT
 *      // H5Z_INTEGER is a C long int
 *      // FLOAT is a C double
 */
/*-------------------------------------------------------------------------
 * Function:    H5Z_unget_token
 * Purpose:     Rollback the H5Z_token to the previous H5Z_token retrieved. There
 *              should only need to be one level of rollback necessary
 *              for our grammar.
 * Return:      Always succeeds.
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
  *              Leon Arber:  Added FUNC_ENTER / FUNC_LEAVE pairs
*
 *-------------------------------------------------------------------------
 */
static void
H5Z_unget_token(H5Z_token *current)
{
    /* check args */
    assert(current);


    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5Z_unget_token)
    
    current->tok_type = current->tok_last_type;
    current->tok_begin = current->tok_last_begin;
    current->tok_end = current->tok_last_end;

    FUNC_LEAVE_NOAPI_VOID
            
    
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_get_token
 * Purpose:     Determine what the next valid H5Z_token is in the expression
 *              string. The current position within the H5Z_token string is
 *              kept internal to the H5Z_token and handled by this and the
 *              unget_H5Z_token function.
 * Return:      Succeess:       The passed in H5Z_token with a valid tok_type
 *                              field.
 *              NULLure:        The passed in H5Z_token but with the tok_type
 *                              field set to ERROR.
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
 *               Leon Arber:  Added FUNC_ENTER / FUNC_LEAVE pairs
 *-------------------------------------------------------------------------
 */
static H5Z_token *
H5Z_get_token(H5Z_token *current)
{
    
    void*         ret_value=current;
    
    FUNC_ENTER_NOAPI(H5Z_get_token, NULL)

    
    /* check args */
    assert(current);

    /* Save the last position for possible ungets */
    current->tok_last_type = current->tok_type;
    current->tok_last_begin = current->tok_begin;
    current->tok_last_end = current->tok_end;

    current->tok_begin = current->tok_end;

    while (current->tok_begin[0] != '\0') {
        if (isspace(current->tok_begin[0])) {
            /* ignore whitespace */
        } else if (isdigit(current->tok_begin[0]) ||
                   current->tok_begin[0] == '.') {
            current->tok_end = current->tok_begin;

            /*
             * H5Z_INTEGER          :=  digit-sequence
             * digit-sequence   :=  digit | digit digit-sequence
             * digit            :=  0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
             */
            if (current->tok_end[0] != '.') {
                /* is number */
                current->tok_type = H5Z_INTEGER;

                while (isdigit(current->tok_end[0]))
                    ++current->tok_end;
            }

            /*
             * float            :=  digit-sequence exponent |
             *                      dotted-digits exponent?
             * dotted-digits    :=  digit-sequence '.' digit-sequence?  |
             *                      '.' digit-sequence
             * exponent         :=  [Ee] [-+]? digit-sequence
             */
            if (current->tok_end[0] == '.' ||
                    current->tok_end[0] == 'e' ||
                    current->tok_end[0] == 'E') {
                current->tok_type = H5Z_FLOAT;

                if (current->tok_end[0] == '.')
                    do {
                        ++current->tok_end;
                    } while (isdigit(current->tok_end[0]));

                if (current->tok_end[0] == 'e' ||
                    current->tok_end[0] == 'E') {
                    ++current->tok_end;

                    if (current->tok_end[0] == '-' ||
                        current->tok_end[0] == '+')
                        ++current->tok_end;

                    if (!isdigit(current->tok_end[0])) {
                        current->tok_type = ERROR;
                        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, current, "Invalidly formatted floating point number")
                    }

                    while (isdigit(current->tok_end[0]))
                        ++current->tok_end;
                }

                /* Check that this is a properly formatted numerical value */
                if (isalpha(current->tok_end[0]) || current->tok_end[0] == '.') {
                    current->tok_type = ERROR;
                    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, current, "Invalidly formatted floating point number")
                }
            }

            break;
        } else if (isalpha(current->tok_begin[0])) {
            /* is symbol */
            current->tok_type = SYMBOL;
            current->tok_end = current->tok_begin;

            while (isalnum(current->tok_end[0]))
                ++current->tok_end;

            break;
        } else {
            /* should be +, -, *, /, (, or ) */
            switch (current->tok_begin[0]) {
                case '+':   current->tok_type = PLUS;    break;
                case '-':   current->tok_type = MINUS;   break;
                case '*':   current->tok_type = MULT;    break;
                case '/':   current->tok_type = DIVIDE;  break;
                case '(':   current->tok_type = LPAREN;  break;
                case ')':   current->tok_type = RPAREN;  break;
                default:
                    current->tok_type = ERROR;
                    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, current, "Unknown H5Z_token in data transform expression ")
            }

            current->tok_end = current->tok_begin + 1;
            break;
        }

        ++current->tok_begin;
    }

    if (current->tok_begin[0] == '\0')
        current->tok_type = END;

    HGOTO_DONE(current);

done:
    FUNC_LEAVE_NOAPI(ret_value)
      
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_xform_destroy_parse_tree
 * Purpose:     Recursively destroys the expression tree.
 * Return:      Nothing
 * Programmer:  Bill Wendling
 *              25. August 2003
 * Modifications:
 *              Leon Arber: Added FUNC_ENTER / FUNC_LEAVE pairs
 *
 *-------------------------------------------------------------------------
 */
void
H5Z_xform_destroy_parse_tree(H5Z_node *tree)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5Z_xform_destroy_parse_tree)
    
    if (!tree)
        return;

    if (tree->type == SYMBOL)
        HDfree(tree->value.sym_val);

    H5Z_xform_destroy_parse_tree(tree->lchild);
    H5Z_xform_destroy_parse_tree(tree->rchild);
    HDfree(tree);
    tree = NULL;

    FUNC_LEAVE_NOAPI_VOID
}


/*-------------------------------------------------------------------------
 * Function:    H5Z_parse
 * Purpose:     Entry function for parsing the expression string.
 * Return:      Success:    Valid H5Z_node ptr to an expression tree.
 *              NULLure:    NULL
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
 *              Leon Arber:  Added FUNC_ENTER / FUNC_LEAVE pairs
 *
 *-------------------------------------------------------------------------
 */
void *
H5Z_xform_parse(const char *expression)
{
    H5Z_token tok;
    void* ret_value;
   
    FUNC_ENTER_NOAPI(H5Z_xform_parse, NULL)
    
    if (!expression)
        return NULL;

    /* Set up the initial H5Z_token for parsing */
    tok.tok_expr = tok.tok_begin = tok.tok_end = expression;
    
    ret_value = (void*)H5Z_parse_expression(&tok);
    
done:
    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_parse_expression
 * Purpose:     Beginning of the recursive descent parser to parse the
 *              expression. An expression is:
 *
 *                  expr     :=  term | term '+' term | term '-' term
 *
 * Return:      Success:    Valid H5Z_node ptr to expression tree
 *              NULLure:    NULL
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
  *              Leon Arber: Added FUNC_ENTER / FUNC_LEAVE pairs
*
 *-------------------------------------------------------------------------
 */
static H5Z_node *
H5Z_parse_expression(H5Z_token *current)
{
    void*         ret_value=NULL;
    
    FUNC_ENTER_NOAPI(H5Z_parse_expression, NULL)

    H5Z_node *expr = H5Z_parse_term(current);

    for (;;) {
        H5Z_node *new_node;
        new_node = NULL;

        current = H5Z_get_token(current);

        switch (current->tok_type) {
        case PLUS:
            new_node = H5Z_new_node(PLUS);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(expr);
                expr = NULL;
                HGOTO_DONE(expr)
            }

            new_node->lchild = expr;
            new_node->rchild = H5Z_parse_term(current);

            if (!new_node->rchild) {
                H5Z_xform_destroy_parse_tree(new_node);
                expr = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            expr = new_node;
            break;
        case MINUS:
            new_node = H5Z_new_node(MINUS);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(expr);
                expr = NULL;
                HGOTO_DONE(expr)
            }

            new_node->lchild = expr;
            new_node->rchild = H5Z_parse_term(current);

            if (!new_node->rchild) {
                H5Z_xform_destroy_parse_tree(new_node);
                expr = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            expr = new_node;
            break;
        case RPAREN:
            H5Z_unget_token(current);
            HGOTO_DONE(expr)
        case END:
            HGOTO_DONE(expr)
        default:
            H5Z_xform_destroy_parse_tree(expr);
            expr = NULL;
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
        }
    }

done:
    FUNC_LEAVE_NOAPI(expr)
        

}

/*-------------------------------------------------------------------------
 * Function:    H5Z_parse_term
 * Purpose:     Parses a term in our expression language. A term is:
 *
 *                  term :=  factor | factor '*' factor | factor '/' factor
 *
 * Return:      Success:    Valid H5Z_node ptr to expression tree
 *              NULLure:    NULL
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
  *              Leon Arber: Added FUNC_ENTER / FUNC_LEAVE pairs
*
 *-------------------------------------------------------------------------
 */
static H5Z_node *
H5Z_parse_term(H5Z_token *current)
{
    H5Z_node *term = NULL;
    void*         ret_value=NULL;
   
    FUNC_ENTER_NOAPI(H5Z_parse_term, NULL);
    term = H5Z_parse_factor(current);
   
    for (;;) {
        H5Z_node *new_node;
        new_node = NULL;
        
        current = H5Z_get_token(current);

        switch (current->tok_type) {
        case MULT:
            new_node = H5Z_new_node(MULT);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(term);
                term = NULL;
                HGOTO_DONE(term)
            }

            new_node->lchild = term;
            new_node->rchild = H5Z_parse_factor(current);

            if (!new_node->rchild) {
                H5Z_xform_destroy_parse_tree(term);
                term = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            term = new_node;
            break;
        case DIVIDE:
            new_node = H5Z_new_node(DIVIDE);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(term);
                term = NULL;
                HGOTO_DONE(term)
            }

            new_node->lchild = term;
            new_node->rchild = H5Z_parse_factor(current);
            term = new_node;

            if (!new_node->rchild) {
                H5Z_xform_destroy_parse_tree(term);
                term = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            break;
        case RPAREN:
            H5Z_unget_token(current);
            HGOTO_DONE(term)
        case END:
            HGOTO_DONE(term)
        default:
            H5Z_unget_token(current);
            HGOTO_DONE(term)
        }
    }

done:
  FUNC_LEAVE_NOAPI(term)

}

/*-------------------------------------------------------------------------
 * Function:    H5Z_parse_factor
 * Purpose:     Parses a factor in our expression language. A factor is:
 *
 *                  factor   :=  number      |  // C long or double
 *                               symbol      |  // C identifier
 *                               '-' factor  |  // unary minus
 *                               '+' factor  |  // unary plus
 *                               '(' expr ')'
 *
 * Return:      Success:    Valid H5Z_node ptr to expression tree
 *              NULLure:    NULL
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
  *              Leon Arber: Added FUNC_ENTER / FUNC_LEAVE pairs
*
 *-------------------------------------------------------------------------
 */
static H5Z_node *
H5Z_parse_factor(H5Z_token *current)
{
    H5Z_node 	*factor=NULL;
    H5Z_node 	*new_node=NULL;
    void*        ret_value=NULL;
   
    FUNC_ENTER_NOAPI(H5Z_parse_factor, NULL);
    
    current = H5Z_get_token(current);

    switch (current->tok_type) {
    case H5Z_INTEGER:
        factor = H5Z_new_node(H5Z_INTEGER);

        if (!factor)
            HGOTO_DONE(factor)

        sscanf(current->tok_begin, "%ld", &factor->value.int_val);
        break;
    case H5Z_FLOAT:
        factor = H5Z_new_node(H5Z_FLOAT);

        if (!factor)
            HGOTO_DONE(factor)

        sscanf(current->tok_begin, "%lf", &factor->value.float_val);
        break;
    case SYMBOL:
        factor = H5Z_new_node(SYMBOL);

        if (!factor)
            HGOTO_DONE(factor)

        factor->value.sym_val = HDcalloc(current->tok_end - current->tok_begin + 1, 1);
        HDstrncpy(factor->value.sym_val, current->tok_begin,
                current->tok_end - current->tok_begin);
        break;
    case LPAREN:
        factor = H5Z_parse_expression(current);

        if (!factor)
            HGOTO_DONE(factor)

        current = H5Z_get_token(current);

        if (current->tok_type != RPAREN) {
            H5Z_xform_destroy_parse_tree(factor);
            factor = NULL;
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Syntax error in data transform expression")
        }

        break;
    case RPAREN:
        /* We shouldn't see a ) right now */
        H5Z_xform_destroy_parse_tree(factor);
        factor = NULL;
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Syntax error: unexpected ')' ")
    case PLUS:
        /* unary + */
        new_node = H5Z_parse_factor(current);

        if (new_node) {
            if (new_node->type != H5Z_INTEGER && new_node->type != H5Z_FLOAT &&
                    new_node->type != SYMBOL) {
                H5Z_xform_destroy_parse_tree(new_node);
                H5Z_xform_destroy_parse_tree(factor);
                factor=NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            factor = new_node;
            new_node = H5Z_new_node(PLUS);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(factor);
                factor = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            new_node->rchild = factor;
            factor = new_node;
        } else {
            H5Z_xform_destroy_parse_tree(factor);
            factor = NULL;
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
        }

        break;
    case MINUS:
        /* unary - */
        new_node = H5Z_parse_factor(current);

        if (new_node) {
            if (new_node->type != H5Z_INTEGER && new_node->type != H5Z_FLOAT &&
                    new_node->type != SYMBOL) {
                H5Z_xform_destroy_parse_tree(new_node);
                H5Z_xform_destroy_parse_tree(factor);
                factor = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            factor = new_node;
            new_node = H5Z_new_node(MINUS);

            if (!new_node) {
                H5Z_xform_destroy_parse_tree(factor);
                factor = NULL;
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
            }

            new_node->rchild = factor;
            factor = new_node;
        } else {
            H5Z_xform_destroy_parse_tree(factor);
            factor = NULL;
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error parsing data transform expression")
        }

        break;
    case END:
        HGOTO_DONE(factor)
    }

done:

    FUNC_LEAVE_NOAPI(factor);
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_new_node
 * Purpose:     Create and initilize a new H5Z_node structure.
 * Return:      Success:    Valid H5Z_node ptr
 *              NULLure:    NULL
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
  *              Leon Arber: Added FUNC_ENTER / FUNC_LEAVE pairs
*
 *-------------------------------------------------------------------------
 */
static H5Z_node *
H5Z_new_node(H5Z_token_type type)
{
    H5Z_node* ret_value;
    H5Z_node* new_node;
    
    FUNC_ENTER_NOAPI(H5Z_new_node, NULL)
    
    new_node = HDcalloc(1, sizeof(H5Z_node));
 
    if (new_node) 
        new_node->type = type;
    else 
        assert(new_node);

done:
        
        FUNC_LEAVE_NOAPI(new_node);
}

#ifdef H5Z_XFORM_DEBUG
/*-------------------------------------------------------------------------
 * Function:    H5Z_XFORM_DEBUG
 * Purpose:     Print out the expression in a nice format which displays
 *              the precedences explicitly with parentheses.
 * Return:      Nothing
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

static void
H5Z_XFORM_DEBUG(H5Z_node *tree)
{
    int i;
    printf("Expression: ");
    H5Z_result res;
    res = H5Z_eval(tree);

    if(res.type == H5Z_INTEGER)
        printf("H5Z_result is: %d",res.value.int_val );
    else if(res.type == H5Z_FLOAT)
        printf("H5Z_result is: %f",res.value.float_val );
    else 
    {
        printf("H5Z_result is ");
        for(i=0; i<5; i++)
            printf("%d ", array[i]);
    }
    printf("\n");
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_print
 * Purpose:     Print out the expression in a nice format which displays
 *              the precedences explicitly with parentheses.
 * Return:      Nothing
 * Programmer:  Bill Wendling
 *              26. August 2003
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
H5Z_print(H5Z_node *tree, FILE *stream)
{
    /* check args */
    assert(stream);

    if (!tree)
        return;

    if (tree->type == H5Z_INTEGER) {
        fprintf(stream, "%ld", tree->value.int_val);
    } else if (tree->type == H5Z_FLOAT) {
        fprintf(stream, "%f", tree->value.float_val);
    } else if (tree->type == SYMBOL) {
        fprintf(stream, "%s", tree->value.sym_val);
    } else {
        fprintf(stream, "(");
        H5Z_print(tree->lchild, stream);

        switch (tree->type) {
        case PLUS:      fprintf(stream, "+"); break;
        case MINUS:     fprintf(stream, "-"); break;
        case MULT:      fprintf(stream, "*"); break;
        case DIVIDE:    fprintf(stream, "/"); break;
        default: fprintf(stream, "Invalid expression tree\n");
            return;
        }

        H5Z_print(tree->rchild, stream);
        fprintf(stream, ")");
    }
}
#endif  /* H5Z_XFORM_DEBUG */


void H5Z_xform_eval(H5Z_node *tree, void* array, hsize_t array_size, hid_t array_type)
{
    unsigned int i;
    int n;  
    float f;
 
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5Z_xform_eval)
      

   if( tree->type == H5Z_INTEGER)
   {
           if(array_type == H5T_NATIVE_INT)
           {
	     n = tree->value.int_val;
               for(i=0; i<array_size; i++)
		 *((int*)array + i) = n;
            
           }
           else if(array_type == H5T_NATIVE_FLOAT)
           {
               f = (float)tree->value.int_val;
               for(i=0; i<array_size; i++)
                   *((float*)array + i) = f;
           }
   }
   else if (tree->type == H5Z_FLOAT)
   {
       for(i=0; i<array_size; i++)
       {
           if(array_type == H5T_NATIVE_INT)
           {
               n = (int)tree->value.float_val;
               for(i=0; i<array_size; i++)
		 *((int*)array + i) = n;
                  
           }
           else if(array_type == H5T_NATIVE_FLOAT)
           {
               f = tree->value.float_val;
               for(i=0; i<array_size; i++)
                   *((float*)array + i) = f;
           }
       }
   }
   else
       H5Z_eval_full(tree, array, array_size, array_type);

    FUNC_LEAVE_NOAPI_VOID 
}
           


/*-------------------------------------------------------------------------
 * Function:    H5Z_eval
 * Return:      Nothing
 * Programmer:  Leon Arber
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5Z_result
H5Z_eval_full(H5Z_node *tree, void* array, hsize_t array_size,  hid_t array_type)
{

    H5Z_result res, resl, resr;
    int n;
    unsigned int i;
    float f;
    H5Z_result ret_value;
    ret_value.type = ERROR;
        
    FUNC_ENTER_NOAPI(H5Z_eval_full, ret_value);
    
    /*if (!tree)
    {
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "tree was NULL")
    }*/

    /* check args */
    assert(tree);

    if (tree->type == H5Z_INTEGER) 
    {
        res.type = H5Z_INTEGER;
        res.value = tree->value;
        HGOTO_DONE(res)
    } 
    else if (tree->type == H5Z_FLOAT) 
    {
        res.type = H5Z_FLOAT;
        res.value = tree->value;
        HGOTO_DONE(res)
    } 
    else if (tree->type == SYMBOL) 
    {
        res.type = SYMBOL;
        res.value = tree->value;
        res.ar_type = array_type;
        HGOTO_DONE(res)
    } 
    else 
    {
        resl = H5Z_eval_full(tree->lchild, array, array_size, array_type);
        resr = H5Z_eval_full(tree->rchild, array, array_size, array_type);
        switch (tree->type) 
        {
        case PLUS:  
           if( (resl.type == SYMBOL) && (resr.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resr.value.int_val + *((int*)array + i);
			*((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resr.value.int_val +*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resl.type == SYMBOL) && (resr.type==H5Z_FLOAT)) 
            {
                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resr.value.float_val + *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resr.value.float_val +*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_INTEGER))
            {
                 res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.int_val + *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.int_val +*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
          
            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_FLOAT))
            {
             
                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.float_val + *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.float_val +*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, ret_value, "Unexpected type conversion operation")

            HGOTO_DONE(res) 
            break;
        case MINUS:
        
           if( (resl.type == SYMBOL) && (resr.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n =  *((int*)array + i) - resr.value.int_val;
                        *((int*)array + i) = n;
                        /* array[i] +=  resr.value.int_val;*/
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f =*((float*)array + i) -  resr.value.int_val;
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resl.type == SYMBOL) && (resr.type==H5Z_FLOAT)) /*we can't upgrade an array w/o allocating more memory, so we downgrade the float_val to an int.*/
            {
                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n =  *((int*)array + i) - resr.value.float_val;
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f =*((float*)array + i) -  resr.value.float_val; 
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.int_val - *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.int_val -*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)

            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_FLOAT))
            {

                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.float_val - *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.float_val -*((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else
                 HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, ret_value, "Unexpected type conversion operation")


            HGOTO_DONE(res) 
            break;


        case MULT:

           if( (resl.type == SYMBOL) && (resr.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resr.value.int_val * *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resr.value.int_val **((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resl.type == SYMBOL) && (resr.type==H5Z_FLOAT)) 
            {
                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resr.value.float_val * *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resr.value.float_val **((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.int_val * *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.int_val **((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)

            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_FLOAT)) 
            {

                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.float_val * *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.float_val **((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else
             HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, ret_value, "Unexpected type operation")

            HGOTO_DONE(res) 
            break;


        case DIVIDE: 

           if( (resl.type == SYMBOL) && (resr.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = *((int*)array + i) / resr.value.int_val; 
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f =*((float*)array + i) / resr.value.int_val ;
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resl.type == SYMBOL) && (resr.type==H5Z_FLOAT)) 
            {
                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = *((int*)array + i) / resr.value.float_val;
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f =*((float*)array + i) / resr.value.float_val;
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_INTEGER))
            {
                res.type = SYMBOL; 
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.int_val / *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.int_val / *((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)

            }
            else if( (resr.type == SYMBOL) && (resl.type==H5Z_FLOAT))
            {

                res.type = SYMBOL;
                for(i=0; i<array_size; i++)
                {
                    if(array_type == H5T_NATIVE_INT)
                    {
                        n = resl.value.float_val / *((int*)array + i);
                        *((int*)array + i) = n;
                    }
                    else if(array_type == H5T_NATIVE_FLOAT)
                    {
                        f = resl.value.float_val / *((float*)array + i);
                        *((float*)array + i) = f;
                    }
                }
                HGOTO_DONE(res)
            }
            else
             HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, ret_value, "Unexpected type operation")

            HGOTO_DONE(res) 
            break;



        default: 
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, ret_value, "Invalid expression tree")
        }

    }

done:
    
    FUNC_LEAVE_NOAPI(res)

}



/*-------------------------------------------------------------------------
 * Function:    H5Z_find_type
 * Return:      Native type of datatype that is passed in
 * Programmer:  Leon Arber, 4/20/04
 * Modifications:
 *                      
 *
 *-------------------------------------------------------------------------
 */
hid_t H5Z_xform_find_type(H5T_t* type)
{
    hid_t          ret_value = SUCCEED;
    
    FUNC_ENTER_NOAPI(H5Z_xform_find_type, FAIL);

    assert(type);

/*    H5T_NATIVE_CHAR         
        H5T_NATIVE_SHORT        
        H5T_NATIVE_INT          
        H5T_NATIVE_LONG         
        H5T_NATIVE_LLONG        

        H5T_NATIVE_UCHAR
        H5T_NATIVE_USHORT
        H5T_NATIVE_UINT
        H5T_NATIVE_ULONG
        H5T_NATIVE_ULLONG

        H5T_NATIVE_FLOAT
        H5T_NATIVE_DOUBLE
        H5T_NATIVE_LDOUBLE
*/
    
    /* Check for SHORT type */
    if((H5T_cmp(type, H5I_object_verify(H5T_NATIVE_SHORT,H5I_DATATYPE) ))==0)
        HGOTO_DONE(H5T_NATIVE_SHORT)

    /* Check for INT type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_INT,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_INT)

 /* Check for LONG type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_LONG,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_LONG)

 /* Check for LONGLONG type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_LLONG,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_LLONG)

 /* Check for UCHAR type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_UCHAR,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_UCHAR)

 /* Check for USHORT type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_USHORT,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_USHORT)

 /* Check for UINT type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_UINT,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_UINT)

 /* Check for ULONG type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_ULONG,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_ULONG)

 /* Check for ULONGLONG type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_ULLONG,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_ULLONG)

 /* Check for FLOAT type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_FLOAT,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_FLOAT)

 /* Check for DOUBLE type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_DOUBLE,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_DOUBLE)


 /* Check for LONGDOUBLE type */
            else if((H5T_cmp(type,  H5I_object_verify(H5T_NATIVE_LDOUBLE,H5I_DATATYPE)))==0)
        HGOTO_DONE(H5T_NATIVE_LDOUBLE)
    else
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "could not find matching type");

    
done:
    FUNC_LEAVE_NOAPI(ret_value)

}

/*-------------------------------------------------------------------------
 * Function:    H5Z_xform_copy_tree
 * Purpose:     Makes a copy of the parse tree passed in.
 * Return:      A pointer to a root for a new parse tree which is a copy
 *              of the one passed in.
 * Programmer:  Leon Arber
 *              April 1, 2004.
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void* H5Z_xform_copy_tree(H5Z_node* tree)
{ 
    H5Z_node* ret_value=NULL;

    FUNC_ENTER_NOAPI(H5Z_xform_copy_tree, NULL)
   
    assert(tree);   

    if(tree->type == H5Z_INTEGER)
    {
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {
            ret_value -> type = H5Z_INTEGER;
            ret_value ->value.int_val = tree->value.int_val;
        }
    }
    else if (tree->type == H5Z_FLOAT)
    {   
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {   
            ret_value -> type = H5Z_FLOAT;
            ret_value ->value.float_val = tree->value.float_val;
        }   
    }
    else if(tree->type == SYMBOL)
    {   
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {   
            ret_value -> type = SYMBOL;
            if ((ret_value->value.sym_val = (char*) HDmalloc(strlen(tree->value.sym_val)+1)) == NULL)
                HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
            else 
                strcpy(ret_value ->value.sym_val, tree->value.sym_val);
        }   
    }   
    else if(tree->type == MULT)
    {
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {
            ret_value->type = MULT;
            ret_value->lchild = (H5Z_node*) H5Z_xform_copy_tree(tree->lchild);
            ret_value->rchild = (H5Z_node*) H5Z_xform_copy_tree(tree->rchild);
        }
    }
    else if(tree->type == PLUS)
    {
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {
            ret_value->type = PLUS;
            ret_value->lchild = (H5Z_node*) H5Z_xform_copy_tree(tree->lchild);
            ret_value->rchild = (H5Z_node*) H5Z_xform_copy_tree(tree->rchild);
        }
    }
    else if(tree->type == MINUS)
    {
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {
            ret_value->type = MINUS;
            ret_value->lchild = (H5Z_node*) H5Z_xform_copy_tree(tree->lchild);
            ret_value->rchild = (H5Z_node*) H5Z_xform_copy_tree(tree->rchild);
        }
    }
    else if(tree->type == DIVIDE)
    {
        if ((ret_value = (H5Z_node*) HDmalloc(sizeof(H5Z_node))) == NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Ran out of memory trying to copy parse tree")
        else
        {
            ret_value->type = DIVIDE;
            ret_value->lchild = (H5Z_node*) H5Z_xform_copy_tree(tree->lchild);
            ret_value->rchild = (H5Z_node*) H5Z_xform_copy_tree(tree->rchild);
        }
    }   
    else
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "Error in parse tree while trying to copy")



            done:
            FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_xform_reduce_tree
 * Purpose:     Simplifies parse tree passed in by performing any obvious
 *              and trivial arithemtic calculations.
 *
 * Return:      None.
 * Programmer:  Leon Arber
 *              April 1, 2004.
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void H5Z_xform_reduce_tree(H5Z_node* tree)
{
  
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5Z_xform_reduce_tree)
 
        if(!tree)
            goto done;

    if((tree->type == PLUS) || (tree->type == DIVIDE) ||(tree->type == MULT) ||(tree->type == MINUS))
    {
        if(((tree->lchild->type == H5Z_INTEGER) || (tree->lchild->type == H5Z_FLOAT)) && ((tree->rchild->type == H5Z_INTEGER) || (tree->rchild->type == H5Z_FLOAT)))
            H5Z_do_op(tree);
        else
        {
            H5Z_xform_reduce_tree(tree->lchild);
            if(((tree->lchild->type == H5Z_INTEGER) || (tree->lchild->type == H5Z_FLOAT)) && ((tree->rchild->type == H5Z_INTEGER) || (tree->rchild->type == H5Z_FLOAT)))
                H5Z_do_op(tree);

            H5Z_xform_reduce_tree(tree->rchild);
            if(((tree->lchild->type == H5Z_INTEGER) || (tree->lchild->type == H5Z_FLOAT)) && ((tree->rchild->type == H5Z_INTEGER) || (tree->rchild->type == H5Z_FLOAT)))
                H5Z_do_op(tree);
        }
    }
    goto done;

done:
    FUNC_LEAVE_NOAPI_VOID;
}

/*-------------------------------------------------------------------------
 * Function:    H5Z_do_op
 * Purpose:     If the root of the tree passed in points to a simple
 *              arithmetic operation and the left and right subtrees are both
 *              integer or floating point values, this function does that
 *              operation, free the left and rigt subtrees, and replaces
 *              the root with the result of the operation.
 * Return:      None.
 * Programmer:  Leon Arber
 *              April 1, 2004.
 * Modifications:
*
 *-------------------------------------------------------------------------
 */
static void H5Z_do_op(H5Z_node* tree)
{
 
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5Z_do_op)
 

    if(tree->type == DIVIDE)
    {
        if((tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_INTEGER;
            tree->value.int_val = tree->lchild->value.int_val / tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if((tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val / tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val / tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.int_val / tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }

    }
    else if(tree->type == MULT)
    {


        if((tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_INTEGER;
            tree->value.int_val = tree->lchild->value.int_val * tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if((tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val * tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val * tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.int_val * tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }

    }
    else if(tree->type == PLUS)
    {


        if((tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_INTEGER;
            tree->value.int_val = tree->lchild->value.int_val + tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if((tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val + tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val + tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.int_val + tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }

    }
    else if(tree->type == MINUS)
    {


        if((tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_INTEGER;
            tree->value.int_val = tree->lchild->value.int_val - tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if((tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val - tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_FLOAT) && (tree->rchild->type==H5Z_INTEGER))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.float_val - tree->rchild->value.int_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
        else if( (tree->lchild->type == H5Z_INTEGER) && (tree->rchild->type == H5Z_FLOAT))
        {
            tree->type = H5Z_FLOAT;
            tree->value.float_val = tree->lchild->value.int_val - tree->rchild->value.float_val;
            HDfree(tree->lchild);
            HDfree(tree->rchild);
            tree->lchild = NULL;
            tree->rchild = NULL;
        }
    }
 
    FUNC_LEAVE_NOAPI_VOID;
              
}
