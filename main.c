
#include <stdio.h>
#include "y.tab.h"
#include "globals.h"		/* global header file */

/*****************************************************************************8*
	Define two table, symbtab and const_tab, to hold symbols and constants.
	Define two counter for temps and labels
	Define boolean for errors
	Define a freelist for allocation.deallocation
*******************************************************************************/

SYMBTABlE  *symbtab;               /* Symbol table */
int    next_tmp ;                        /* Count of temporaries */
int    next_label ;                      /* Count of labels */
SYMBTABLE  *const_tab;            /* Small constants */
int    errors_found ;                    /* True if we have any errors */
SYMB  *symb_list ;                       /* Freelists */

#define CONST_MAX  10
#define LAB_MIN   10

void main( int argc, char *argv[] ) ;
void init( ) ;
SYMB  *mkconst( int  n ) ;
SYMB  *mklabel( int  l ) ;
SYMB  *mktmp( void ) ;
SYMB  *get_symb( void ) ;
void   free_symb( SYMB *s ) ;
void  *safe_malloc( int  n ) ;
void   insert( SYMB *s ) ;
int    hash( char *s ) ;
SYMB  *lookup( char *s ) ;
void dump_table(  ) ;
extern int yyparse(void);
void   error( char *str ) ;

int main( int argc, char* argv[] ) {

	/****************************************
		YYPARSE returns 0 if parsing was 
		succesful due to end of file
	****************************************/
	
	/*used for debugging
	int i;
	extern int yydebug;
    yydebug = 1;*/
	
	/* lexer check 
	while ( i = yylex() ) 
		printf ( "Token %d ", i);*/

	if ( yyparse() == 0 ) printf( "\n\n Parsing Passes \n\n" );
	
	else printf( "\n\n Parsing failed \n\n" );
	
	return 0;
	
}


void init ( ) {

	int  i ;                             /* General counter */


    symb_list   = NULL ;                 /* Freelists */
    
    errors_found = FALSE ;               /* No errors found yet */
    next_tmp     = CONST_MAX ;                   /* No temporaries used */
    next_label   = LAB_MIN ;             		 /* 10 labels reserved */

    for( i = 0 ; i < HASHSIZE ; i++ )    /* Clear symbol table */
        symbtab->symbtab[i] = NULL ;
	
	/* Make constants 
		numbers 1-10 set into the tabel as constants */
	for( i = 0 ; i < CONST_MAX ; i++ )   
        {
            SYMB *c = get_symb() ;       /* symbol cor constant */

			c->type = T_INT;			/* const type is an int */	
			c->val = i;					/* const val is 1-10 */	
			c->token_type1 = T_CONSTANT;	/* const token */
			c->token_type2 = T_CONSTANT;
			c->offset = 0;
			c->size = 4;
			
			const_tab->symbtab[i] = c;	/* enter xonst symbol into table */
        }

}

SYMB *mkconst( int  n ) {

	/* symb is already in table */
    if((n >= 0) && (n < CONST_MAX))
            return const_tab->symbtab[n] ;
    else
    {
        SYMB *c = get_symb() ;   /* Create a new node */

        c->type = T_INT ;
        c->val = n ;
		c->token_type1 = T_CONSTANT;	/* const token */
		c->token_type2 = T_CONSTANT;
		c->offset = 0;
		c->size = 4;
        return c ;
    }

}







