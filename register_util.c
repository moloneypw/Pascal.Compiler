#include <stdlib.h>
#include <stdio.h>
#include "register_util.h"

int  get_rreg( SYMB *c ) {

	int r ;                   /* Register for counting */

	for( r = R_EAX ; r < R_MAX ; r++ )   /* Already in a register */
		if( rdesc[r].name == c ) {
			//fprintf( "\nSpill rreg %s\n", c->text );
			spill_one( r ) ;
			return r ;
		}

	for( r = R_EAX ; r < R_MAX ; r++ )   /* Empty register */
		if( rdesc[r].name == NULL ) {
			load_reg( r, c ) ;
			return r ;
		}

        for( r = R_EAX ; r < R_MAX ; r++ )
                if( !rdesc[r].modified )     /* Unmodifed register */
                {
                        clear_desc( r ) ;
                        load_reg( r, c ) ;
                        return r ;
                }

        spill_one( R_EAX ) ;                 /* Modified register */
        clear_desc( R_EAX ) ;
        load_reg( R_EAX, c ) ;
        return R_EAX ;

}       /* int  get_rreg( SYMB *c ) */


int  get_areg( SYMB *b, int cr )  {

        int        r ;                   /* Register for counting */

        for( r = R_EBP ; r < R_MAX ; r++ )
                if( rdesc[r].name == b )              /* Already in register */
                        return r ;

        for( r = R_EAX ; r < R_MAX ; r++ )
                if( rdesc[r].name == NULL )           /* Empty register */
                {
                        load_reg( r, b ) ;
                        return r ;
                }

        for( r = R_EAX ; r < R_MAX ; r++ )
                if( !rdesc[r].modified && (r != cr))  /* Unmodifed register */
                {
                        clear_desc( r ) ;
                        load_reg( r, b ) ;
                        return r ;
                }

        for( r = R_EAX ; r < R_MAX ; r++ )
                if( r != cr )                          /* Modified register */ {
                        spill_one( r ) ;
                        clear_desc( r ) ;
                        load_reg( r, b ) ;
                        return r ;
                }

} 

void  load_reg( int r, SYMB *n ) {
				
        switch( n->token_type2 )
        {
        case T_CONSTANT:

                fprintf( file_out, "\tmovl     $%d, %s", n->val, get_reg_string( r ) ) ;
	fprintf( file_out, "\t\t# move constant into a register\n" );
                break ;

        case T_VAR:

                fprintf( file_out, "\tmovl     %d(%s), %s", 0-n->base_ptr_offset, "%ebp", get_reg_string( r ) ) ;
	fprintf( file_out, "\t\t# move variable into a register\n" );
                break ;

        case T_TEXT:

                fprintf( file_out, "       LDA  L%u,R%u\n", n->offset, r ) ;
                break ;
				
       case T_TEMP:
		
	fprintf( file_out, "\tmovl     %d(%s), %s", 0-n->base_ptr_offset, "%ebp", get_reg_string( r ) ) ;
	fprintf( file_out, "\t\t# move temp variable into a register\n" );
                break ;	  
        }
        insert_desc( r, n, UNMODIFIED ) ;
} 

void  spill_all( void ) {
        int  r ;

        for( r = R_EAX ; r < R_MAX ; r++ )
                spill_one( r ) ;
} 



void  spill_one( int  r ){ 
	if( (rdesc[r].name != NULL) && rdesc[r].modified ) {
		fprintf( file_out, "\t%s     %s, %d(%s)", "movl", get_reg_string ( r ), 0-rdesc[r].name->base_ptr_offset, "%ebp" ) ;
		fprintf( file_out, "\t\t# move value into proper spot on stack \n");
		rdesc[r].modified = UNMODIFIED ;
	}
}


void  flush_all( void  ){

        int  r ;

        spill_all() ;

        for( r = R_EAX ; r < R_MAX ; r++ )   /* Clear the descriptors */
                clear_desc( r ) ;

        clear_desc( R_EAX ) ;                /* Clear result register */

} 

char* get_reg_string ( int r ) {
	
	switch ( r ) {
	
	case R_EAX:
		return "%eax";
	case R_EBX:
		return "%ebx";
	case R_ECX:
		return "%ecx";
	case R_EDX:
		return "%edx";
	}
}
