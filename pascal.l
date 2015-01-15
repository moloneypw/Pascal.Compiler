#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "globals.h"    /* holds global valus and declarations */
#include "y.tab.h"      /* header file with all token values */
void yyerror(char *);   /* print errors */
extern YYSTYPE yylval ;   /* YYSTPE */
#define token(x) x      /* return token value */

%}

/*******************
  DEFINITIONS
*******************/

alpha     [a-zA-Z_]
alphanum    [a-zA-Z_0-9]
unsignedint     [0-9]+
whitespace    [ \t\n]
comment     "{"[^}]*"}"
string      \"([^"\n]|\\["\n])*\"
exponent    [eE][+-]?[0-9]+

%%

"AND"               {return token(AND);}
"ARRAY"             {return token(ARRAY);}
"BEGIN"       {return token(_BEGIN);}
"CASE"              {return token(CASE);}
"DIV"               {return token(DIV);}
"DO"                {return token(DO);}
"ELSE"              {return token(ELSE);}
"END"               {return token(END);}
"FOR"               {return token(FOR);}
"FORWARD"           {return token(FORWARD);}
"FUNCTION"          {return token(FUNCTION);}
"IF"                {return token(IF);}
"INTEGER"     {return token(INTEGER);}
"MOD"               {return token(MOD);}
"NOT"               {return token(NOT);}
"OF"                {return token(OF);}
"OR"                {return token(OR);}
"PROCEDURE"         {return token(PROCEDURE);}
"PROGRAM"           {return token(PROGRAM);}
"PRINT"       {return token(PRINT);}
"PRINTS"      {return token(PRINTS);}
"REAL"        {return token(REAL);}
"REPEAT"            {return token(REPEAT);}
"RETURN"      {return token(RETURN);}
"THEN"              {return token(THEN);}
"UNTIL"             {return token(UNTIL);}
"VAR"               {return token(VAR);}
"WHILE"             {return token(WHILE);}
">="        {return token(GE);}
">"         {return token(GT);}
"<="        {return token(LE);}
"<"         {return token(LT);}
"=="          {return token(EQ);}
"<>"        {return token(NE);}
":="        {return token(ASSIGN);}
".."        {return token(DOTDOT);}
{alpha}{alphanum}*    { mkname ( yytext );    return token(ID);}  
{unsignedint}     { mkval ();   return token(NUM_INT);}     
{unsignedint}("."{unsignedint})?    { mkval (); return token(NUM_REAL);   
{unsignedint}{exponent}?        { mkval (); return token(NUM_REAL);}    
{string}        { mktext ();  return token(STRING);}  
{whitespace}+               ;
{comment}+                          ;         
.   {return token(yytext[0]);}

%%

int yywrap(void) {
  return 1;
}

void  mkname( char *name ) {
        struct symb *t ;                 /* Pointer to looked up symbol */
        char        *s ;                 /* Permanent text of string */
        int          len ;               /* Length of text */

    if ( declaration_mode == 0 && parameter_mode == 0 ) {
    if ( (t = lookup( name )) != NULL ) {       
      if ( hops != 0 ) {          /* symb is not local */
        SYMB *temp;
        //temp =  t;
        temp = get_symb();
        temp->text = t->text;
        temp->token_type1 = T_NONLOCAL;
        temp->token_type2 = T_NONLOCAL;
        temp->type = T_NONLOCAL;
        temp->val = 0;
        temp->offset = 0;
        temp->hops = hops;
        temp->size = 0;
        temp->c = 0;
        
        insert ( temp );
        yylval.symb = temp;
        return ;
        
      }     
      /* symbol is found locally - return it. */
      yylval.symb = t;
      return ;
    }
    }

        /* Unseen before, so make a permanent copy, set up a new symbol table
           node, insert it into the symbol table and assign it to yylval. */
        len = strlen( name ) ;
        s   = (char *)safe_malloc( len + 1 ) ; /* Space for perm copy */

        strncpy( s, name, len ) ;
        s[len] = '\0' ;

        t        = get_symb() ;
        t->type  = T_UNDEF ;             /* Don't know which node type yet */
        t->token_type2 = T_UNDEF;
    t->text = s ;                   /* Text argument */
    t->c = 0;

        insert( t ) ;
        yylval.symb = t ;                /* Symbol type in union */

}       /* void  mkname( char *name ) */


void  mkval( void ) {
        yylval.symb = mkconst( yytext ) ;   
}     

SYMB *mkconst( char* name ) {

  SYMB *t;
  int len;
  char* s;
  
  t = get_symb(); 
  
  len = strlen( name ) ;
    s   = (char *)safe_malloc( len + 1 ) ; /* Space for perm copy */
    strncpy( s, name, len ) ;
    s[len] = '\0' ; 
  t->text = s;
  
  t->token_type1 = T_CONSTANT;
  t->token_type2 = T_CONSTANT;
  t->type = T_INT;
  t->size = 4;
  t->dimension = 0;
  t->offset = 0;
  t->hops = 0;
  t->c = 0;
  t->val = atoi( name );
  
  insert_const( t );
  
  return t;
}
void  mktext( void ) {
        mkname( yytext ) ;
        yylval.symb->type = T_TEXT ;
}  
