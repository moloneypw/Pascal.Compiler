#include <stdio.h>
#include "y.tab.h"
#include "globals.h"		/* global header file */

/*****************************************************************************8*
	Define two table, symbtab and const_tab, to hold symbols and constants.
	Define two counter for temps and labels
	Define boolean for errors
	Define a freelist for allocation.deallocation
*******************************************************************************/

SYMBTABLE  *symbtab;               /* Symbol table */

int main( int argc, char *argv[] ) ;
void dump_table( SYMBTABLE *symbtab ) ;
extern int yyparse(void);
void   error( char *str ) ;
char *type_string( int t ) ;

int main( int argc, char* argv[] ) {

	/****************************************
		YYPARSE returns 0 if parsing was 
		succesful due to end of file
	****************************************/
	
	/*used for debugging
	int x;
	extern int yydebug;
   	 yydebug = 1;*/
	
	/* lexer check 
	while ( i = yylex() ) 
		printf ( "Token %d ", i);*/
		
	printf( "\n#################");
	printf( "\n#    PARSING    #");
	printf( "\n#################\n");

	if ( yyparse() == 0 ) printf( "\nParsing Passes \n\n" );
	
	else printf( "\n\n Parsing failed \n\n" );
	
	printf( "\n#################");
	printf( "\n#  TABLE DUMP   #");
	printf( "\n#################");	
	
	int i;
	
	for ( i = 0; i < stack_counter; i++ ){
		dump_table( table_stack[i] );
	}
	
	dump_table( const_symb_table );
	
	printf( "\n\n#################");
	printf( "\n#   TAC DUMP    #");
	printf( "\n#################\n");
	
	tac_list = reverse_tac ( tac_list );
	
	TAC* temp_list;
	temp_list = tac_list;
	
	while ( temp_list != NULL ) {
		print_TAC ( temp_list );
		temp_list = temp_list->next;
	}
	
	printf( "\n\n######################");
	printf( "\n#   CODE GENERATION    #");
	printf( "\n########################\n");
	
	printf( "\nOut assembly code found in file t.s\n" );
	
	cg ( tac_list );
	
	printf("\n\n");
	
	return 0;
	
}

/**********************************8
	DUMP TABLE
	Prints out the contents of a symbol table.
***********************************/
void dump_table ( SYMBTABLE *symbtab ) {

		int i;
		SYMB *t;
		
		//printf("\nTABLE DUMP\n\n");
		printf("\n\nSYMBOL TABLE NAME: %s \t WIDTH %d \t PARAMTERS %d \n", symbtab->name, symbtab->width, symbtab->parameters );
		
		printf( "\tSYMBOL NAME \t TOKEN TYPE 1 \t TOKEN TYPE 2 \t\t DATA TYPE \t SIZE \t\t OFFSET \t DIMENSION \t HOPS \t C \n" );
			
		for ( i = 0; i < HASHSIZE; i++ ){
			t = symbtab->symbtab[i];
			while ( t != NULL ) {		
				printf( "\t%s \t\t %s \t %s \t\t %s \t %d \t\t %d \t\t %d \t\t %d \t %d\n" ,
					t->text, type_string( t->token_type1 ), type_string( t->token_type2 ), type_string( t->type ), t->size, t->offset, t->dimension, t->hops, t->c );
				t = t->next;
			}
		}
}






/*****************************************
	TYPE STRING
	Takes a type and prints out the string.
*****************************************/
char *type_string( int t ){
	
	switch ( t ) {	
	case T_UNDEF:
		return "T_UNDEF";		
	case T_VAR:
		return "T_VAR ";			
	case T_FUNC:
		return "T_FUNC";			
	case T_TEXT:
		return "T_TEXT";			
	case T_INT:
		return "T_INT ";			
	case T_LABEL:
		return "T_LABEL";			
	case T_LOCAL:
		return "T_LOCAL";			
	case T_SELF:
		return "T_SELF";			
	case T_NONLOCAL:
		return "T_NONLOCAL";			
	case T_PROGRAM:
		return "T_PROGRAM";			
	case T_PARAM:
		return "T_PARAM";	
	case T_FILE:
		return "T_FILE";		
	case T_CONSTANT:
		return "T_CONSTANT";	
	case T_REAL:
		return "T_REAL";		
	case T_ARRAY:
		return "T_ARRAY";		
	case T_PROC:
		return "T_PROC";	
	default:
		break;
	}
}
