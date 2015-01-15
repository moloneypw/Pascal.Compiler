%{
/********************************************/
/* Yacc - pascal.y	                     	*/
/* Author - Patrick Moloney		         	*/
/* Date - 2/6/12							*/
/* Step 2									*/
/*											*/
/* .. definitions .. 						*/
/* %%										*/
/* .. rules ..								*/
/* %%										*/
/* .. subroutines ..						*/
/*											*/
/* New things added for step 2				*/
/* -Synthesized Attributes					*/
/* -allow for multple nested levels			*/
/* -Multi-dimensional arrays				*/
/*											*/
/********************************************/

#include <stdio.h>
int yylex(void);
void yyerror(char *);		/* error handling */

#define YYDEBUG 1			/* for debugging  */

%}
 
/***************
	TOKENS
***************/

%token INTEGER ID STRING REAL INT
%token AND ARRAY _BEGIN CASE DIV DO
%token ELSE END FOR FORWARD FUNCTION
%token IF MOD NOT OF OR
%token PROCEDURE PROGRAM REAL REPEAT
%token THEN UNTIL VAR WHILE
%token GE GT LE LT EQ NE ASSIGN DOTDOT
%token UMINUS

%left  '+' '-'
%left  '*' '/'
%right UMINUS

%%

/******************
	DEFINITIONS
******************/

program_start:
	PROGRAM ID '(' identifier_list ')' ';'
	declarations
	subprogram_declarations
	compound_statement
	'.'
	;
	
identifier_list:
	ID
	| identifier_list ',' ID
	;

/* synthesized attributes */
/* change i_l ':' type -> d_i_l	*/
declarations:
	declarations VAR decl_identifier_list ';'
	|         
	;
	
/* synthesized attributes - step 2 */
decl_identifier_list:
	ID decl_identifier_list
	| ',' ID decl_identifier_list
	| ':' type
	;

type:
	standard_type
	| ARRAY '[' array_expression_list ']' OF standard_type		/* handle UMInus */
	;
	
/* expression list for array to handle - */
array_expression_list:
	'-' INT DOTDOT INT  %prec UMINUS
	|  array_expression_list ',' INT DOTDOT INT   		/* mutli dimensional array */
	|  INT DOTDOT INT
	;
	
standard_type:
	INTEGER
	| REAL
	;
	
subprogram_declarations:
	subprogram_declarations subprogram_declaration ';'
	|  
	;
	
/* multi level nesteing */
subprogram_declaration: 
	subprogram_head
	declarations
	subprogram_declarations 			/* to allow for nesting */
	compound_statement
	;
	
subprogram_head:
	FUNCTION ID arguments ':' standard_type ';'
	| PROCEDURE ID arguments ';'
	;
	
arguments:
	'(' parameter_list ')'
	|  
	;
	
/* synthesized attributes for a parameter list */	
parameter_list:
	param_identifier_list
	| parameter_list ';' param_identifier_list
	;
	
/* synthesized attruibutes for param_identifier list */
param_identifier_list:
	ID param_identifier_list
	| ',' ID param_identifier_list
	| ':' type
	;
	

compound_statement:
	_BEGIN
	optional_statements
	END
	;
	
optional_statements:
	statement_list
	|  
	;

statement_list:
	statement
	| statement_list ';' statement
	;
	
statement:
	variable ASSIGN expression
	| procedure_statement
	| compound_statement
	| IF expression THEN statement ELSE statement
	| WHILE expression DO statement
	;
	
variable:
	ID
	| ID '[' expression_list ']' 	/* mutli diemnsional array */
	;
	
procedure_statement:
	ID
	| ID '(' expression_list ')'
	;
	
expression_list:
	expression
	| expression_list ',' expression
	;
	
expression:
	simple_expression
	| simple_expression GE simple_expression
	| simple_expression LE simple_expression
	| simple_expression GT simple_expression
	| simple_expression LT simple_expression
	| simple_expression EQ simple_expression
	| simple_expression NE simple_expression
	;
	
simple_expression:
	term
	| sign term
	| simple_expression '+' term
	| simple_expression '-' term
	| simple_expression OR term
	;
	
term:
	factor
	| term '*' factor
	| term '/' factor
	| term DIV factor
	| term MOD factor
	| term AND factor
	;
	
factor:
	ID
	| ID '(' expression_list ')'
	| multi_dimen_array ']'
	| INT
	| REAL
	| '(' expression ')'
	| NOT factor
	
	;

/* mutli dimensional array expression */
multi_dimen_array:
	 multi_dimen_array ',' INT 
	|  ID '['  INT
	;
	
sign:
	'+'
	| '-'
	;
	

%%

/* print out the error */
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}



