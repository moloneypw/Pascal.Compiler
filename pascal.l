%{
/********************************/
/* Lexical Analyzer - pascal.l	*/
/* Author - Patrick Moloney		*/
/* Date - 2/19/12				*/
/* Step 3 - Simple Symbol Table	*/
/*								*/
/* .. definitions .. 			*/
/* %%							*/
/* .. rules ..					*/
/* %%							*/
/* .. subroutines ..			*/
/*                              */
/* Funtiona added to create     */
/* sumbols for various tokesn   */
/*                              */
/********************************/

#include <stdlib.h>
void yyerror(char *);		/* print errors */
#include "globals.h"		/* holds global valus and declarations */
#include "y.tab.h"			/* header file with all token values */

#define token(x) x			/* return token value */

%}

/*******************
	DEFINITIONS
*******************/

alpha			[a-zA-Z_]
alphanum		[a-zA-Z_0-9]
unsignedint 	[0-9]+
whitespace 		[ \t\n]
comment 		"{"[^}]*"}"
string 			\"([^"\n]|\\["\n])*\"
exponent 		[eE][+-]?[0-9]+

%%

"AND"               {return token(AND);}
"ARRAY"             {return token(ARRAY);}
"BEGIN" 			{return token(_BEGIN);}
"CASE"              {return token(CASE);}
"DIV"               {return token(DIV);}
"DO"                {return token(DO);}
"ELSE"              {return token(ELSE);}
"END"               {return token(END);}
"FOR"               {return token(FOR);}
"FORWARD"           {return token(FORWARD);}
"FUNCTION"          {return token(FUNCTION);}
"IF"                {return token(IF);}
"INTEGER"			{return token(INTEGER);}
"MOD"               {return token(MOD);}
"NOT"               {return token(NOT);}
"OF"                {return token(OF);}
"OR"                {return token(OR);}
"PROCEDURE"        	{return token(PROCEDURE);}
"PROGRAM"           {return token(PROGRAM);}
"REAL"				{return token(REAL);}
"REPEAT"            {return token(REPEAT);}
"THEN"              {return token(THEN);}
"UNTIL"             {return token(UNTIL);}
"VAR"               {return token(VAR);}
"WHILE"             {return token(WHILE);}

">=" 				{return token(GE);}
">"					{return token(GT);}
"<="				{return token(LE);}
"<"					{return token(LT);}
"="					{return token(EQ);}
"<>"				{return token(NE);}
":="				{return token(ASSIGN);}
".."				{return token(DOTDOT);}

{alpha}{alphanum}*							{ mkname ( yytext );
												return token(ID);}			/* make variable */	
{unsignedint}								{ mkval ();
												return token(INT);}			/* make value */
{unsignedint}\.{unsignedint}({exponent})?   { mkval ();
												return token(REAL);}		/* make value */
{unsignedint}{exponent}?    				{ mkval ();
												return token(REAL);}		/* make value */
{string}									{ mktext ();
												return token(STRING);}		/* make text */
{whitespace}+								;
{comment}+                 					;					
.											{return token(yytext[0]);}


%%

/* reached EOF terminate */
int yywrap(void) {
	return 1;
}

void  mkname( char *name )

/* Lookup a name in the symbol table. If not there set up a new one, returning
   the result in "yylval". Note that "yylval" is a union, defined in parser.h
   and so we must specify which member of the union (in this case "symb") we
   wish to used. We insert the name in the symbol table, but at this stage with
   type T_UNDEF, since we do not know what its type is. */

{
        struct symb *t ;                 /* Pointer to looked up symbol */
        char        *s ;                 /* Permanent text of string */
        int          len ;               /* Length of text */


        /* If the string exists return with it in yylval */

        if((t = lookup( name )) != NULL )
        {
                yylval.symb = t ;
                return ;
        }

        /* Unseen before, so make a permanent copy, set up a new symbol table
           node, insert it into the symbol table and assign it to yylval. */

        len = strlen( name ) ;
        s   = (char *)safe_malloc( len + 1 ) ; /* Space for perm copy */

        strncpy( s, name, len ) ;
        s[len] = EOS ;

        t        = get_symb() ;
        t->type  = T_UNDEF ;             /* Don't know which node type yet */
        t->text = s ;                   /* Text argument */

        insert( t ) ;
        yylval.symb = t ;                /* Symbol type in union */

}       /* void  mkname( char *name ) */


void  mkval( void )

/* Set up a node for an integer read by the scanner. We use the library routine
   "atoi()" to convert the text of "yytext" to an integer. */

{
        yylval.symb = mkconst( atoi( yytext )) ;

}       /* void  mkval( void ) */


void  mktext( void )

/* Text is treated very similarly to a variable name. "mktext()" uses
   "mkname()" to do most of the work, and then just changes the relevant
   fields. Strings will be put out as constant data at the end of the program
   and we give it a label to mark its beginning. Note that there is no risk of
   a piece of text being mistaken for a variable name during symbol lookup,
   since the text field of a text node includes the surrounding quotes. */

{
        mkname( yytext ) ;
        yylval.symb->type = T_TEXT ;
        //yylval.symb->VAL2 = next_label++ ;

}       /* mktext( void ) */








