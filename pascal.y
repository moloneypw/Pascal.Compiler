#include <stdio.h>
#include "globals.h"
int yylex(void);			/* lexer		  */
void yyerror(char *);		/* error handling */

#define YYDEBUG 1			/* for debugging  */

%}
  
%union {
	struct symb		*symb ;          
	struct dim_t 	*dim;
	struct tac  	*tac ;            /* For most things */
	struct enode  	*enode ;          /* For expressions */
}
 
/***************
	TOKENS
***************/

%token <symb> ID 
%token <symb> NUM_REAL 
%token <symb> NUM_INT
%token <symb> STRING 
%token INTEGER REAL
%token AND ARRAY _BEGIN CASE DIV DO
%token ELSE END FOR FORWARD FUNCTION
%token IF MOD NOT OF OR
%token PROCEDURE PROGRAM PRINT REAL REPEAT RETURN
%token THEN UNTIL VAR WHILE PRINTS
%token GE GT LE LT EQ NE ASSIGN DOTDOT
%token UMINUS

%left  '+' '-'
%left  '*' '/'
%right UMINUS

%type <dim> array_expression_list

%type <symb> decl_identifier_list
%type <symb> type
%type <symb> standard_type
%type <symb> param_identifier_list
%type <symb> subprogram_decl_identifier_list 
%type <symb> variable

%type <enode> expression_list
%type <enode> expression
%type <enode> simple_expression
%type <enode> term
%type <enode> factor
%type <enode> proc_argument_list

%type <tac> statement
%type <tac> program_start
%type <tac> identifier_list
%type <tac> compound_statement
%type <tac> declarations
%type <tac> statement_list
%type <tac> optional_statements
%type <tac> subprogram_declaration
%type <tac> subprogram_declarations
%type <tac> cond_statement
%type <tac> subprogram_head
%type <tac> arguments
%type <tac> subprogram_var_declarations
%type <tac> procedure_statement
%type <tac> parameter_list

%%

/******************
	DEFINITIONS
******************/

program_start:
	M1
	PROGRAM 
	ID
		{
		main_symb_table->name = $3->text;
		$3->token_type1 = T_SELF;
		$3->token_type2 = T_PROGRAM;
		$3->offset = 0;
		$3->hops = 0;
		$3->size = 0;
		$3->parent = main_symb_table;
		}
	'(' identifier_list ')' ';'
	declarations
	subprogram_declarations
	compound_statement
	'.'
		{ $$ = do_func ( $3, $6, $9, $10, $11 );  }
	;
	
M1:		/* empty - initialize */
	{
	const_symb_table = mktable ( NULL ) ;
	table_ptr = mktable ( NULL );
	
	main_symb_table = table_ptr;
	stack_counter = 0;
	table_stack[stack_counter] = main_symb_table;
	stack_counter++;
	main_symb_table->width = 0;
	
	int len = strlen( "CONSTANT SYMBOL TABLE" ) ;
    char *s = (char *)safe_malloc( len + 1 ) ; 
    strncpy( s, "CONSTANT SYMBOL TABLE", len ) ;
    s[len] = '\0' ;
	const_symb_table->name = s;
	
	offset = 0;	
	main_symb_table->width = 0;
	
	constant_offset = 0;
	const_symb_table->width = 0;
	
	parameter_mode = 1;			/* turn on parameters 	*/
	declaration_mode = 1;		/* turn on declarations */
	tac_list = NULL;			/* initialize tac list 	*/
	enode_list = NULL;			/* initialize enodes	*/
	decl_tac_list = NULL;		/* init decl tac list	*/
	code_tac_list = NULL;
	next_tmp = 0;				/* set temp to 0		*/
	next_label = 10;			/* 10 labels reserved	*/
	string_counter = 2;
	string_list = NULL;
	
	int z;
	for ( z = 0; z < 100; z ++ ) {
		true_list[z] = (int)NULL;
		false_list[z] = (int)NULL;
	}	
		
	}
	;
	
identifier_list:
	ID
		{
		
		/* create a new T_VAR TAC and join it to the tac list */
		$$ = declare_var( $1 ); 
		//tac_list = join_tac( t, tac_list ); 

		$1->token_type1 = T_PARAM;
		$1->token_type2 = T_FILE;
		$1->offset = 0;
		$1->hops = 0;
		$1->size = 0;
		main_symb_table->parameters++;
		}
	| identifier_list ',' ID
		{
		
		$$ = join_tac ( $1, declare_var ( $3 ) );
		
		$3->token_type1 = T_PARAM;
		$3->token_type2 = T_FILE;
		$3->offset = 0;
		$3->hops = 0;
		$3->size = 0;
		main_symb_table->parameters++;
		}
	|	/* empty identifier list */
	;

/* synthesized attributes */
/* change i_l ':' type -> d_i_l	*/
declarations:
	declarations VAR decl_identifier_list ';'	{ $$ = decl_tac_list; }
	|     										{ $$ = NULL; }    
	;
	
decl_identifier_list:
	ID 
		{
		/* create a new T_VAR TAC and join it to the tac list */
		TAC *t = declare_var( $1 ); 
		decl_tac_list = join_tac( t, decl_tac_list ); 
		}		
	decl_identifier_list		
		{
		$1->token_type1 = T_LOCAL;
		$1->token_type2 = $3->token_type2;
		$1->type = $3->type;
		$1->offset = offset;
		
		if ( $1->token_type2 == T_ARRAY ) {
			$1->c = $3->c;
			}	
			
		$1->size = $3->size;
		main_symb_table->width = main_symb_table->width + $3->size;
		offset = offset + $3->size;
		}
	| ',' 
	  ID 
		{
		/* create a new T_VAR TAC and join it to the tac list */
		TAC *t = declare_var( $2 ); 
		decl_tac_list = join_tac( t, decl_tac_list ); 
		}	
	  decl_identifier_list
		{
		
		$2->token_type1 = T_LOCAL;
		$2->token_type2 = T_VAR;
		$2->type = $4->type;
		$$->type = $4->type;		
		$2->offset = offset;
		
		
		
		if ( $2->token_type2 == T_ARRAY ) {
			$2->c = offset - temp_c;
		}
		
		main_symb_table->width = main_symb_table->width + $4->size;
		offset = offset + $4->size;
		$2->size = $4->size;
		$$->size = $4->size;		
		}
	| ':' type
		{ $$ = $2; }
	;

type:
	standard_type	
		{ 
		$$->token_type2 = T_VAR;
		$$->type = $1->type;
		$$->size = 4;
		}
	| ARRAY '[' array_expression_list ']' OF standard_type		/* handle UMInus */
		{ 
		$$->token_type2 = T_ARRAY;
		$$->type = $6->type;
		$$->offset = $3->offset;
		$$->size = $3->size * 4;
		$$->dimension = $3->dimension;
		
		$$->c = $3->offset - $3->c;
		

		
		}
	;
	
/* expression list for array to handle - */
array_expression_list:
	'-' NUM_INT DOTDOT NUM_INT  %prec UMINUS
		{
		$$->size = ($4->val + $2->val) + 1;
		$$->dimension++;
		
		$2->offset = constant_offset;
		constant_offset = constant_offset + 4;
		$4->offset = constant_offset;
		const_symb_table->width = constant_offset;
		
		$2->type = T_INT;
		$4->type = T_INT;
		}
	|  array_expression_list ',' NUM_INT DOTDOT NUM_INT   		
		{		
		$$->size = $1->size * (($5->val - $3->val) + 1);
		$$->dimension++;
		
		$3->offset = constant_offset;
		constant_offset = constant_offset + 4;
		
		$5->offset = constant_offset;
		constant_offset = constant_offset + 4;
		
		const_symb_table->width = constant_offset;
		
		$3->type = T_INT;
		$5->type = T_INT;
		
		temp_c = (temp_c * $5->val) - ( $3->val * 4);

		}
	|  NUM_INT DOTDOT NUM_INT
		{
		$$->size = ($3->val - $1->val) + 1;
		$$->dimension++;
		
		$1->offset = constant_offset;
		constant_offset = constant_offset + 4;
		
		$3->offset = constant_offset;
		constant_offset = constant_offset + 4;
		
		const_symb_table->width = constant_offset;	
		
		$1->type = T_INT;
		$3->type = T_INT;
		
		$$->lower_bound = $1->val;
		
		$$->c = $1->val * 4;
		}
	;
	
standard_type:
	INTEGER { $$->type = T_INT; }
	| REAL  { $$->type = T_REAL; }
	;
	
subprogram_declarations:
	subprogram_declarations
	subprogram_declaration { table_ptr = table_ptr->parent; }
	';'
		{ $$ = join_tac( $1, $2 ); }
	|   { $$ = NULL; }
	;
	
/* multi level nesteing */
subprogram_declaration: 
	subprogram_head
	subprogram_var_declarations
	subprogram_declarations 			/* to allow for nesting */
	compound_statement
		{ $$ = do_subprogram_func( $1, $2, $3, $4 ); }
	;
	
subprogram_head:
	 FUNCTION ID M2 arguments ':' standard_type ';'
		{
		
		//function inserted into parent table
		$2->token_type1 = T_LOCAL;
		$2->token_type2 = T_FUNC;
		table_ptr->name = $2->text;
		//$2->type = $6->type;
		$2->nested_table = table_ptr;
		
		//new symbol made and insereted into its own table
		mkname ( $2->text );
		yylval.symb->parent = table_ptr->parent;
		yylval.symb->token_type1 = T_SELF;
		yylval.symb->token_type2 = T_FUNC;
		yylval.symb->type = $6->type;
		
		$$ = do_subprogram_head( $2, $4 ) ;
		}
	
	| PROCEDURE ID M2 arguments ';' 
		{
		
		//function inserted into parent table
		$2->token_type1 = T_LOCAL;
		$2->token_type2 = T_PROC;
		table_ptr->name = $2->text;
		$2->nested_table = table_ptr;
		
		//new symbol made and insereted into its own table
		mkname ( $2->text );
		yylval.symb->parent = table_ptr->parent;
		yylval.symb->token_type1 = T_SELF;
		yylval.symb->token_type2 = T_PROC;
		
		$$ = do_subprogram_head( $2, $4 );
		}
	;
	
M2:	
	{
	table_ptr = mktable ( table_ptr );
	table_stack[stack_counter] = table_ptr;
	stack_counter++;
	table_ptr->width = 0;
	table_ptr->parameters = 0;
	offset = 0;
	
	decl_tac_list = NULL;
	param_tac_list = NULL;
	}
	;
	
/* synthesized attributes */
/* change i_l ':' type -> d_i_l	*/
subprogram_var_declarations:
	subprogram_var_declarations VAR subprogram_decl_identifier_list ';' { $$ = decl_tac_list; }
	|         
	;
	
/* synthesized attributes - step 2 */
subprogram_decl_identifier_list:
	ID 
		{
		/* create a new T_VAR TAC and join it to the tac list */
		TAC *t = declare_var( $1 ); 
		decl_tac_list = join_tac( t, decl_tac_list ); 
		}	
	subprogram_decl_identifier_list
		{
		$1->token_type1 = T_LOCAL;
		$1->token_type2 = $3->token_type2;
		$1->type = $3->type;
		$1->offset = offset;
		offset = offset + $3->size;
		$1->size = $3->size;
		table_ptr->width = table_ptr->width + $3->size;
		}
	| ',' ID 
		{
		/* create a new T_VAR TAC and join it to the tac list */
		TAC *t = declare_var( $2 ); 
		decl_tac_list = join_tac( t, decl_tac_list ); 
		}	
		subprogram_decl_identifier_list
		{
			$2->token_type1 = T_LOCAL;
			$2->token_type2 = $4->token_type2;
			$$->token_type2 = $4->token_type2;
			$2->type = $4->type;
			$$->type = $4->type;
			$2->offset = offset;
			offset = offset + $4->size;
			$2->size = $4->size;
			$$->size = $4->size;
			table_ptr->width = table_ptr->width + $4->size;
		}
	| ':' type
		{ $$ = $2; }
	;
	
	
arguments:
	'(' parameter_list ')'	{ $$ = $2 ; }
	|  						{ $$ = NULL ;}
	;
	
/* synthesized attributes for a parameter list */	
parameter_list:
	param_identifier_list							{ $$ = param_tac_list ;}
	| parameter_list ';' param_identifier_list
	;
	
/* synthesized attruibutes for param_identifier list */
param_identifier_list:
	ID param_identifier_list
		{
		param_tac_list = join_tac( declare_var( $1 ), param_tac_list );
		
		$1->token_type1 = T_PARAM;
		$1->token_type2 = T_VAR;
		$1->type = $2->type;
		$1->size = $2->size;
		$1->offset
 = offset;
		
		offset = offset + $2->size;
		table_ptr->width = table_ptr->width + $2->size;
		table_ptr->parameters++;
		
		
		}
	| ',' ID param_identifier_list
		{
		param_tac_list = join_tac( declare_var( $2 ), param_tac_list );
		
		$2->token_type1 = T_PARAM;
		$2->token_type2 = T_VAR;
		$2->type = $3->type;
		$2->size = $3->size;
		$2->offset = offset;
		
		offset = offset + $3->size;
		table_ptr->parameters++;
		table_ptr->width = table_ptr->width + $3->size;
		
		$$->size = $3->size;
		$$->type = $3->type;		
		}
	| ':' type
		{
		$$ = $2;
		}
	;
	

compound_statement:
	_BEGIN
		{parameter_mode = 0; declaration_mode = 0;}		/* turn declaration and parameter mode off */
	optional_statements	//{  code_tac_list = join_tac( $3, code_tac_list ); }
	END
		{parameter_mode = 1; declaration_mode = 1; $$ = $3; }		/* turn declaration and parameter mode on */
	;
	
optional_statements:
	statement_list { $$ = $1; }
	|  
	;

statement_list:
	statement		{ $$ = $1; 					}	
	| statement_list ';' statement  { $$ = join_tac( $1, $3 ); 	}
	;
	
statement:
	variable ASSIGN expression 			{ $$ = do_assign( $1, $3 );		}
	| procedure_statement				{ $$ = $1;						}
	//| compound_statement
	| cond_statement
	| IF expression THEN statement ELSE statement	{ $$ = do_if( $2, $4, $6); 		}
	| WHILE expression DO statement			{ $$ = do_while( $2, $4 ); 		}
	| RETURN expression				{ $$ = do_return( $2 );			}
	| PRINT '(' expression ')' 				{ $$ = do_func_print ( $3 );	}
	| PRINTS '(' STRING ')'				{ $3->string_num = string_counter;
							  string_counter++;
							  mkstring( $3 );	
							  $$ = do_print_string( $3 );	}
	;
	

cond_statement:
	_BEGIN
	statement_list	
	END
	{ $$ = $2; }
	;
	
variable:
	ID
	| ID '[' expression ']' 	{ $$ = var_array( $1, $3 ); }
	;
	
procedure_statement:
	ID									
	| ID '(' proc_argument_list ')'		{ $3 = do_fnap( $1, $3 ); $$ = $3->tac; }
	;
	
proc_argument_list:
	expression_list			{ $$ = $1;		}
	| /* empty arg list */	{ $$ = NULL; 	}
	;
	
expression_list:
	expression 							{ $$ = $1;					}
	| expression_list ',' expression 	{ $3->next = $1; $$ = $3;	}
	;
	
expression:
	simple_expression 							{ $$ = $1;  								}
	| simple_expression GE simple_expression	{ $$ = do_relop( GREATERTHANEQ, $1, $3 ); 	}
	| simple_expression LE simple_expression	{ $$ = do_relop( LESSTHANEQ, $1, $3 ); 		}
	| simple_expression GT simple_expression	{ $$ = do_relop( GREATERTHAN, $1, $3 );		}
	| simple_expression LT simple_expression	{ $$ = do_relop( LESSTHAN, $1, $3 );		}
	| simple_expression EQ simple_expression	{ $$ = do_relop( EQUAL, $1, $3 ); 			}
	| simple_expression NE simple_expression	{ $$ = do_relop( NOTEQ, $1, $3 );			}
	;
	
simple_expression:
	term 							{ $$ = $1; }
	| '-' term	%prec UMINUS		{ $$ = do_un( TAC_NEG , $2 ); }
	| simple_expression '+' term	{ $$ = do_bin( TAC_ADD, $1, $3 ); }
	| simple_expression '-' term	{ $$ = do_bin( TAC_SUB, $1, $3 ); }
	| simple_expression OR term		{ $$ = do_or( $1, $3 ); }
	;
	
term:
	factor				{ $$ = $1; }
	| term '*' factor 	{ $$ = do_bin( TAC_MUL, $1, $3 ); }
	| term '/' factor 	{ $$ = do_bin( TAC_DIV, $1, $3 ); }
	| term DIV factor
	| term MOD factor
	| term AND factor	{ $$ = do_and( $1, $3 ); }
	;
	
factor:
	ID 									{ $$ = mkenode( NULL, $1, NULL );   }
	| ID '(' expression_list ')'		{ $$ = do_fnap( $1, $3 );  			}
	| ID '[' factor ']'					{ $$ = do_array( $1, $3 ); 			}
	| NUM_INT	
		{ 
		$1->type = T_INT;  
		$1->token_type1 = T_CONSTANT; 
		$1->token_type2 = T_CONSTANT; 
		$1->offset = constant_offset;
		constant_offset = constant_offset + 4;
		const_symb_table->width = constant_offset;
		
										  $$ = mkenode( NULL, $1, NULL );
		}			
	| NUM_REAL	
		{ 
		$1->type = T_REAL; 
		$1->token_type1 = T_CONSTANT; 
		$1->token_type2 = T_CONSTANT; 
		$1->offset = constant_offset;
		constant_offset = constant_offset + 4;
		const_symb_table->width = constant_offset;
		
											$$ = mkenode( NULL, $1, NULL );
		}			
	| '(' expression ')'				{ $$ = $2; }
	| NOT factor
	;
%%

/*************************
	SUBROUTINEs
*************************/

/* print out the error */
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}






SYMB *mktmp( void ) {

        SYMB *old_yylval = yylval.symb ;         /* Save the old yylval */
        SYMB *tmp ;                              /* Used to restore result */
        char name[12] ;                         /* For the name of the var */
		char *s ;                 /* Permanent text of string */
        int len ;               /* Length of text */

        /* Make the name with mkname */

        sprintf( name, "T%d", next_tmp++ ) ;   /* Set up text */
        //mkname( name ) ;
		
		len = strlen( name ) ;
        s = (char *)safe_malloc( len + 1 ) ; /* Space for perm copy */

        strncpy( s, name, len ) ;
        s[len] = '\0' ;

        yylval.symb = get_symb() ;
		yylval.symb->text = s;
        yylval.symb->type = T_VAR ;
		yylval.symb->token_type2 = T_TEMP;

        /* Hang onto this new symbol in tmp, restore yylval.symb and return
           tmp */ 

        tmp         = yylval.symb ;
        yylval.symb = old_yylval ;

        return tmp ;

} 
