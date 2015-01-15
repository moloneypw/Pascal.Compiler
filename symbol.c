#include "globals.h"

SYMB *mklabel( int  l ){
        SYMB *t = get_symb() ;

        t->type = T_LABEL ;
        t->val = l ;

        return t ;
}

SYMB* var_array( SYMB* s, ENODE* expr ) {
	
	TAC* temp;
	TAC* copy;
	TAC* array;
	
	temp = mktac( TAC_VAR, mktmp(), NULL, NULL );
	
	int i = s->c + ( expr->res->val * 4 );
	char *index[10];
	sprintf( (char*) index, "%d", i );		
	
	copy = mktac( TAC_COPY, (SYMB *) temp->varA, mkconst( (char*) index ), NULL );
	
	copy->prev = temp;
	
	s->tac_array = copy;
	
	return s;	
}


SYMB *lookup( char *s ) {

	hops = 0;						/* set hops */
        	int   hv = hash( s ) ;				/* hash value */
        	SYMB *t  = table_ptr->symbtab[hv] ;	/* get symb from pos in current table */
	SYMBTABLE *temp = table_ptr;;		/* set a temp table to be the current table */	
		
		while ( temp != NULL ) {				
			t  = temp->symbtab[hv];			
			while( t != NULL ) {   
					if( strcmp( t->text, s ) == 0 ) {
							return t; ;
						}
					else
						t = t->next ;
			}
			
			temp = temp->parent;
			hops++;
			}
        return t ;                       /* NULL if not found */}


void *safe_malloc( int  n ) {

        void *t = (void *)malloc( n ) ;

        if( t == NULL )   {
                error( "malloc() failed" ) ;
                exit( 0 ) ;
        }

        return t ;
}


SYMB *get_symb( void ) {

        SYMB *t ;

        t = (SYMB *)safe_malloc( sizeof( SYMB )) ;

        return t ;

} 





void  insert_const( SYMB *s ) {

		int hv = s->val % HASHSIZE;

		if ( const_symb_table->symbtab[hv] == NULL ) {
			s->next = const_symb_table->symbtab[hv] ;          /* Insert at head */
			const_symb_table->symbtab[hv]  = s ;
			return;
		}
		
		if ( strcmp(s->text, const_symb_table->symbtab[hv]->text ) == 0 ){
			constant_offset = constant_offset -4;
			return;
		}
		
		s->next = const_symb_table->symbtab[hv] ;          /* Insert at head */
		const_symb_table->symbtab[hv]  = s ;
		
}

void  insert( SYMB *s ) {
        int hv = hash( s->text ) ;		/* get hash value */

        s->next = table_ptr->symbtab[hv] ;          /* Insert at head */
        table_ptr->symbtab[hv]  = s ;
}



int  hash( char *s ) {

        int  hv = 0 ;
        int  i ;

        for( i = 0 ; s[i] != '\0' ; i++ )
        {
                int  v = (hv >> 28) ^ (s[i] & 0xf) ;

                hv = (hv << 4) | v ;
        }

        hv = hv & 0x7fffffff ;           /* Ensure positive */
        return hv % HASHSIZE ;

}      

SYMBTABLE *mktable ( SYMBTABLE *parent ) {

	SYMBTABLE *new_t;
	
	new_t = (SYMBTABLE *)malloc( sizeof ( SYMBTABLE ));
	
	/* initailize values */
	new_t->parent = parent;
	new_t->next = NULL;
	new_t->width = 0;
	new_t->parameters = 0;
	new_t->name = "";
	
	/* set table entries to NULL */
	int i;
	for ( i = 0; i < HASHSIZE; i++ ) {
		new_t->symbtab[i] = NULL;
	}
	
	return new_t;
	
}
