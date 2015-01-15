/*************************************************************
	
	GLOBALS.H
	
	Header file that conatins 
	constants, structs, global variables, and routine
	used thoughout the program.
	
	Vatious fragments of code copied from Bennett
	
************************************************************/

/***************
	CONSTANTS
***************/

#define TRUE        1                    /* Booleans */
#define FALSE       0
#define HASHSIZE  997                    /* Size of symbol table */

/* Tags for symbols to know what they are */
#define  T_UNDEF  0                      /* Types for symbol table */
#define  T_VAR    1                      /* Local Variable */
#define  T_FUNC   2                      /* Function */
#define  T_TEXT   3                      /* Static string */
#define  T_INT    4                      /* Integer constant */
#define  T_LABEL  5                      /* TAC label */

/**************
	STRUCTS
**************/

/* Stucture for a Symbol which is then inserted in the Symbol Table. */	

typedef struct symb                      
{
	struct symbol_table *parent		/* table symbol belongs to */
	struct symbol_table *next		/* pointer to nested symbole table */
	struct symb *next;  /* Next in chain */
    int type;       	/* What is the data type of this symbol */
    int val;        	/* For integers */
	char *text;			/* For var names */
	int offset;			/* offset in arrays */
	int depth;			/* how many levels nested */
	int size;			/* size of type being stored */
	int token_type1;	/* where is the token located in the program*/
	int token_type2;	/* type of token */

} SYMB ;

/* Structure for Symbol Table.
	Used to hold an array of SYMB's. */
	
typedef struct symbol_table                      
{
	struct symbol_table *parent		/* table symbol belongs to */
	struct symbol_table *next		/* pointer to nested symbole table */
    int width;       	/* toatl bytes stored */
    int parameters;     /* total parameters of program */
	char *name;			/* name of table */
	SYMB *symbtab[HASHSIZE]; 	/*symbol table; holds an array of symbols */

} SYMBTABLE ;

/************************************
	GLOBAL VARIBALES / ROUTINES
************************************/

extern SYMBTABLE *symbtab;         /* Symbol table */
extern int   next_tmp ;                  /* Count of temporaries */
extern int   next_label ;                /* Count of labels */

/* main.c */
extern SYMB  *mkconst( int  n ) ;        
extern SYMB  *mklabel( int  l ) ;
extern SYMB  *mktmp( void ) ;
extern void   insert( SYMB *s ) ;
extern SYMB  *lookup( char *s ) ;
extern SYMB  *get_symb( void ) ;
extern void   free_symb( SYMB *s ) ;
extern void  *safe_malloc( int  n ) ;

/* pascal.l */
extern void  mkname( char *name ) ;     
extern void  mkval( void );
extern void  mktext( void );