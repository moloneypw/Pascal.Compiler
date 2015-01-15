include <stdlib.h>
#include <stdio.h>
#include "tac.h"

TAC *mktac( int op, SYMB *a, SYMB *b, SYMB *c ) {

	TAC *t = (TAC *)safe_malloc( sizeof( TAC )) ;

	t->next = NULL ;                /* Set these for safety */
	t->prev = NULL ;
	t->op = op ;
	
	t->varA = a ;
	t->varB = b ;
	t->varC = c ;

	return t ;
} 

TAC *join_tac( TAC *c1, TAC *c2 ) {

	TAC *t ;
	
	/* If either list is NULL return the other */
	if( c1 == NULL )	{ return c2 ; }
	if( c2 == NULL )	{ return c1 ; }
		
	/* Run down c2, until we get to the beginning and then add c1 */
	t = c2 ;

	while( t->prev != NULL )	{ t = t->prev; }	/* go until previous of c2 is NULL */

	t->prev = c1 ;	/* add c1 to prev of c2's list */
	
	return c2 ;	
} 
	
void  print_TAC( TAC *i ) {
        fflush( stdout ) ;

}

TAC *declare_var( SYMB *var ) {		
        
        var->token_type2  = T_VAR ;
        var->offset = -1 ;                /* Unset address */

        /* TAC for a declaration */

        return  mktac( TAC_VAR, var, NULL, NULL ) ;

} 




TAC *do_assign( SYMB  *var,   ENODE *expr )   {

        TAC  *code ;

        /* Warn if variable not declared, then build code */

        if( var->type != T_INT && var->token_type2 != T_ARRAY )
                error( "assignment to non-variable" ) ;				

	if ( var->token_type2 == T_ARRAY ) {
		printf( "\n%d\n", var->tac_array->varB->val );
		code = mktac( TAC_COPY, var, expr->res, var->tac_array->varB ) ;	}
        	else
		code = mktac( TAC_COPY, var, expr->res, NULL ) ;
		
        	expr->tac = join_tac( var->tac_array, expr->tac );
	code->prev = expr->tac ;
       	 free_enode( expr ) ;             /* Expression now finished with */

        return code ;
}

TAC *do_func( SYMB *func, TAC  *args, TAC *decls, TAC *subs, TAC  *code ) {

        TAC *tlist ;                     /* The backpatch list */

        TAC *tlab ;                      /* Label at start of function */
        TAC *tbegin ;                    /* BEGINFUNC marker */
        TAC *tend ;                      /* ENDFUNC marker */

        if( func->type != T_UNDEF )       {
                yyerror( "function already declared" ) ;
                return NULL ;
        }

        tlab   = mktac( TAC_BEGIN_LABEL, 	func, NULL, NULL ) ;
        tbegin = mktac( TAC_BEGINFUNC, 		func, NULL, NULL ) ;
        tend   = mktac( TAC_ENDFUNC,   		NULL, NULL, NULL ) ;
		
        	tbegin->prev = tlab ;	
	code 		 = join_tac( decls,	code	) ; 
      	  code         = join_tac( args, 	code  	) ;
       	 tend->prev   = join_tac( tbegin, code 	) ;

		
	subs = join_tac( tend, subs   );
		
        tlist = func->branch_label ;                   /* List of addresses if any */
		
		

        while( tlist != NULL )
        {
                TAC *tnext = tlist->labB ;         /* Next on list */

                tlist->labB  = tlab ;
                tlist      = tnext ;
        }
		
		

        func->token_type2   = T_FUNC ;          /* And declare as func */
        func->branch_label = tlab ;

	tac_list = join_tac( subs , tac_list );		
	tac_list = join_tac( string_list, tac_list );
		
        return subs ;

}


TAC *reverse_tac( TAC* tl ){

	TAC *c;
	TAC *p;

	c = NULL ;                       /* No current */
	p = tl ;                         /* Preceding to do */

	while( p != NULL ) {
		p->next = c ;            /* Set the next field */
		c = p ;            /* Step on */
		p = p->prev ;
	}

	return c;

}


TAC *do_func_print ( ENODE  *args ){


	TAC *a;		/* arguments */
	TAC *print;
	
	a = mktac ( TAC_ARG, args->res, NULL, NULL );
	print = mktac ( TAC_PRINT, NULL, NULL, NULL );
	
	a->prev = args->tac;
	
	print->prev = a;
	
	return print;
	
}

TAC* do_return( ENODE* expr ) {
	TAC* ret;	
	ret = mktac( TAC_RETURN, expr->res, NULL, NULL );	
	ret->prev = NULL;	
	return ret;
}

TAC* do_print_string( SYMB* string ) {

	TAC* ts;
	TAC* print;
	
	ts = mktac( TAC_ARG_STRING, string, NULL, NULL );
	print = mktac ( TAC_PRINT_STRING, NULL, NULL, NULL );
	
	ts->prev = NULL;
	print->prev = ts;
	
	return print;
}

TAC *do_if( ENODE *expr, TAC *stmt1, TAC* stmt2 ) {
		
		TAC *label = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL ) ;
		TAC * label2 = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL );
        		TAC *code  = mktac( TAC_IFZ, (SYMB *)label->varA , expr->res, NULL ) ;
		TAC *go    = mktac( TAC_GOTO, (SYMB *)label2->varA, NULL, NULL );
		
		go->prev = NULL;
		label2->prev = NULL;

     		code->prev  = expr->tac ;
		stmt1 = join_tac( stmt1, go );
       		code        = join_tac( code, stmt1 ) ;
        		label->prev = code ;

		label = join_tac( label, stmt2  );
		
		label = join_tac( label, label2 );
		
       		 free_enode( expr ) ;             

        		return label ;
}

TAC* do_while( ENODE* expr, TAC* stmt ){
	
	TAC* label_begin;
	TAC* label_end;
	TAC* code;
	TAC* goto_begin;	

	label_begin = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL );
	label_end = mktac( TAC_LABEL, mklabel( next_label++ ), NULL, NULL );	
	code = mktac( TAC_IFZ, (SYMB*) label_end->varA, expr->res, NULL );
	goto_begin = mktac( TAC_GOTO, (SYMB*) label_begin->varA, NULL, NULL );
	
	label_begin = join_tac( label_begin, expr->tac );
	code->prev = label_begin;
	goto_begin->prev = join_tac( code, stmt );
	label_end->prev = goto_begin;	
	return label_end;
}

TAC* do_subprogram_head( SYMB* fname, TAC* args ) {


	/* set up the called fucntion */
	
	TAC* sbegin_label;
	TAC* sbegin_func;
	
	sbegin_label = mktac( TAC_SBEGIN_LABEL, fname, NULL, NULL );
	sbegin_func = mktac( TAC_SBEGIN_FUNC, fname, NULL, NULL );
	
	sbegin_label->prev = NULL;
	sbegin_func->prev = sbegin_label;
	
	args = join_tac( sbegin_func, args );
	
	return args;
	
}

TAC* do_subprogram_func( TAC* sbegin, TAC* decls, TAC* nested_func, TAC* code ) {

	TAC* end_label;
	
	
	end_label = mktac( TAC_ENDSFUNC, NULL, NULL, NULL );
	
	
	code = join_tac( decls, code );
	code = join_tac( sbegin, code );
	end_label->prev = code;
	
	end_label = join_tac( end_label, nested_func );

	return end_label;

}

void mkstring( SYMB* string ){

	TAC* ts;	
	ts = mktac( TAC_PRINTS, string, NULL, NULL );	
	string_list = join_tac( string_list, ts );
}

void  print_TAC_CG( TAC *i ) {
        fflush( stdout ) ;
}
