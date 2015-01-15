#include "enode.h"

ENODE *mkenode( ENODE *next, SYMB *res, TAC *code ) {

	ENODE *expr = get_enode() ;

	expr->next = next ;
	expr->res  = res ;
	expr->tac  = code ;

	return expr ;
		
}

void  free_enode( ENODE *expr ) {

	expr->next = enode_list ;
	enode_list = expr ;

} 

ENODE *get_enode( void ) {

	if( enode_list != NULL ) {
		ENODE *expr ;

		expr = enode_list ;
		enode_list = expr->next ;

		return expr ;
	}
	else
		return (ENODE *)safe_malloc( sizeof( ENODE )) ;

} 


ENODE *do_fnap( SYMB  *func, ENODE *arglist ) {

        ENODE  *alt ;                    /* For counting args */
        SYMB   *res ;                    /* Where function result will go */
        TAC    *code ;                   /* Resulting code */
        TAC    *temp ;                   /* Temporary for building code */

        if(( func->type != T_UNDEF ) && ( func->token_type2 != T_FUNC || func->token_type2 != T_PROC))
        {
                error( "function declared other than function" );
                return NULL ;
        }

        res   = mktmp() ;                            /* For the result */
        code  = mktac( TAC_VAR, res, NULL, NULL ) ;

        for( alt = arglist ; alt != NULL ; alt = alt->next )  /* Join args */
                code = join_tac( code, alt->tac ) ;

        while( arglist != NULL )         /* Generate ARG instructions */
        {
                temp       = mktac( TAC_ARG, arglist->res, NULL, NULL ) ;
                temp->prev = code ;
                code       = temp ;

                alt = arglist->next ;
                free_enode( arglist ) ;  /* Free the space */
                arglist = alt ;
        } ;

        temp       = mktac( TAC_CALL, res, (SYMB *)func, NULL ) ;
        temp->prev = code ;
        code       = temp ;

        return mkenode( NULL, func, code ) ;

} 



ENODE *do_bin( int binop, ENODE *expr1, ENODE *expr2 ) {

        TAC  *temp ;                     /* TAC code for temp symbol */
        TAC  *res ;                      /* TAC code for result */

	if(( expr1->res->token_type2 == T_CONSTANT ) && ( expr2->res->token_type2 == T_CONSTANT))
        {
                int   newval;            /* The result of constant folding */

                switch( binop )          /* Chose the operator */
                {
                case TAC_ADD:

                        newval = expr1->res->val + expr2->res->val ;
                        break ;

                case TAC_SUB:

                        newval = expr1->res->val - expr2->res->val ;
                        break ;

                case TAC_MUL:

                        newval = expr1->res->val * expr2->res->val ;
                        break ;

                case TAC_DIV:

                        newval = expr1->res->val / expr2->res->val ;
                        break ;
                }


				char *newv[10];
				sprintf( (char*) newv, "%d", newval );
				
                expr1->res = mkconst( (char*) newv );  /* New space for result */
                free_enode( expr2 ) ;          /* Release space in expr2 */
				
                return expr1 ;             /* The new expression */
        }

        temp       = mktac( TAC_VAR, mktmp(), NULL, NULL ) ;
        temp->prev = join_tac( expr1->tac, expr2->tac ) ;
        res        = mktac( binop, temp->varA, expr1->res, expr2->res ) ;
		
        res->prev  = temp ;		

        expr1->res = temp->varA ;
        expr1->tac = res ;
        free_enode( expr2 ) ;
		
        return expr1 ;  

}

ENODE *do_un( int unop, ENODE *expr ) {

        TAC  *temp ;                     /* TAC code for temp symbol */
        TAC  *res ;                      /* TAC code for result */

        /* Do constant folding if possible. Calculate the constant into expr */

        if( expr->res->token_type2 == T_CONSTANT )
        {
                switch( unop )           /* Chose the operator */
                {
                case TAC_NEG:

                        expr->res->val = 0 - expr->res->val ;
                        break ;
                }

                return expr ;              /* The new expression */
        }

        /* Not constant, so create a TAC node for a unary operator, putting
           the result in a temporary. Bolt the code together, reusing expr. */

        temp       = mktac( TAC_VAR, mktmp(), NULL, NULL ) ;
        temp->prev = expr->tac ;
        res        = mktac( unop, temp->varA, expr->res, NULL  ) ;
        res->prev  = temp ;

        expr->res = temp->varA ;
        expr->tac = res ;

        return expr ;   
} 

ENODE *do_array( SYMB* s, ENODE* expr1 ) {

	TAC* width;
	TAC* index;
	TAC* wi;
	TAC* val;
	
	TAC* t1;
	TAC* t2;
	TAC* t3;
	TAC* t4;	

	char *four[10];
	sprintf( (char*) four, "%d", 4 );
	
	t1 = mktac( TAC_VAR, mktmp(), NULL, NULL );
	t2 = mktac( TAC_VAR, mktmp(), NULL, NULL );
	t2->prev = t1;

	width = mktac( TAC_COPY, t1->varA, mkconst( (char*) four ), NULL );
	index = mktac( TAC_COPY, t2->varA, expr1->res, NULL );
	width->prev = t2;
	index->prev = width;
	
	t3 = mktac( TAC_VAR, mktmp(), NULL, NULL );
	wi = mktac( TAC_MUL, t3->varA, width->varA, index->varA );
	t3->prev = index;
	wi->prev = t3;
	wi->varA->val = s->c + (4 * expr1->res->val);
	
	t4 = mktac( TAC_VAR, mktmp(), NULL, NULL );
	val = mktac( TAC_ARRAY, t4->varA, wi->varA, s);
	t4->prev = wi;
	val->prev = t4;
	
	expr1->tac = join_tac( expr1->tac, val );	
	
	t4->varA->val = wi->varA->val;
	expr1->res = t4->varA;
	
	return expr1;	

}

ENODE *do_relop( int rel_op, ENODE* expr1, ENODE* expr2 ) {

	int op;
	
	op = rel_op;	/* op hold the realtional operator */
	
	TAC* if_stmt;
	TAC* copy_true;
	TAC* copy_false;
	TAC* goto_label;
	TAC* temp;
	TAC* const_temp;
	
	TAC* label_true;
	TAC* label_false;
	
	char *zero[10];
	char *one[10];
	sprintf( (char*)one, "%d", 1 );
	sprintf( (char*)zero, "%d", 0 );
	
	temp = mktac( TAC_VAR, mktmp(), NULL, NULL );
	
	label_true = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL );
	
	switch ( op )	{
	
	case EQUAL:
		if_stmt = mktac( TAC_EQUAL, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	case NOTEQ:
		if_stmt = mktac( TAC_NE, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	case GREATERTHAN:
		if_stmt = mktac( TAC_GT, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	case GREATERTHANEQ:
		if_stmt = mktac( TAC_GTE, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	case LESSTHAN:
		if_stmt = mktac( TAC_LT, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	case LESSTHANEQ:
		if_stmt = mktac( TAC_LTE, (SYMB*) label_true->varA , expr1->res, expr2->res );
		break;
	}
	
	
	copy_false = mktac( TAC_COPY, temp->varA, mkconst( (char*) zero ), NULL );
	
	label_false = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL );
	goto_label = mktac( TAC_GOTO, ( SYMB * )label_false->varA , NULL, NULL );
	
	copy_true = mktac( TAC_COPY, temp->varA, mkconst( (char*) one ), NULL );
	
	temp->prev = join_tac( expr1->tac, expr2->tac );
	if_stmt->prev = temp;
	copy_false->prev = if_stmt;
	goto_label->prev = copy_false;
	label_true->prev = goto_label;
	copy_true->prev = label_true;
	label_false->prev = copy_true;
	
	
	expr1->tac = label_false;
	expr1->res = temp->varA;

	
	free_enode( expr2 );
	
	return expr1;

}

ENODE* do_or( ENODE* expr1, ENODE* expr2 ){

	TAC* temp;
	TAC* res;
	
	temp = mktac( TAC_VAR, mktmp(), NULL, NULL );
	
	/* temp->val = 0, 1, or 2.  1 and 2 are TRUE, 0 is FALSE */
	res = mktac( TAC_ADD, temp->varA, expr1->res, expr2->res );	
	
	expr1->res = temp->varA;
	
	temp->prev = join_tac( expr1->tac, expr2->tac );
	res->prev = temp;
	expr1->tac = res;
	

	return expr1;

}

ENODE* do_and( ENODE* expr1, ENODE* expr2 ) {

	TAC* temp;
	TAC* cmp;
	TAC* copy;
	
	ENODE* num;
	
	char *zero[10];
	char *two[10];
	sprintf( (char*)two, "%d", 2 );
	sprintf( (char*)zero, "%d", 0 );
	
	num = mkenode( NULL, mkconst( (char*) two ), NULL );
	
	temp = mktac( TAC_VAR, mktmp(), NULL, NULL );
	copy = mktac( TAC_ADD, temp->varA, expr1->res, expr2->res );
	
	expr1->res = temp->varA;
	
	temp->prev = join_tac( expr1->tac, expr2->tac );
	copy->prev = temp;
	expr1->tac = copy;
	
	return do_relop( LESSTHANEQ, num, expr1 );	
}	
