#include <stdlib.h>
#include <stdio.h>
#include "cg.h"

void  cg( TAC *tl ) {

	TAC *tls = init_cg( tl ) ;              /* Start of TAC */

	for( ; tls != NULL ; tls = tls->next )  /* Instructions in turn */
	{
		fprintf( file_out, "# " ) ;
		print_TAC_CG( tls ) ;
		cg_instr( tls ) ;
	}
}  

TAC *init_cg( TAC *tl ) {

	int  r ;
	TAC *c ;                         /* Current TAC instruction */
	TAC *p ;                         /* Previous TAC instruction */

	for( r = 0 ; r < R_MAX ; r++ )
		rdesc[r].name = NULL ;

	sprintf( (char*) zero, "%d", 0 );
	insert_desc( 0, mkconst((char*)  zero ), UNMODIFIED ) ;     /* R0 holds 0 */

	tos      = VAR_OFF ;             /* TOS allows space for link info */
	next_arg = 0 ;                   /* Next arg to load */

	file_out = fopen( "t.s", "w" );
	
	return tl ;

}

void  insert_desc( int r, SYMB *n, int mod ) {
				   
    int  or ;                       /* Old register counter */

    for( or = R_EBP ; or < R_MAX ; or++ )
    {
        if( rdesc[or].name == n )
        {
                /* Found it, clear it and break out of the loop. */

                clear_desc( or ) ;
                break ;
        }
    }

    /* We should not find any duplicates, but check, just in case. */

    for( or++ ; or < R_MAX ; or++ )
    
            if( rdesc[or].name == n )
            {
                    error( "Duplicate slave found" ) ;
                    clear_desc( or ) ;
            }

    /* Finally insert the name in the new descriptor */

    rdesc[r].name     = n ;
    rdesc[r].modified = mod ;

}

/***********************************
	CLEAR DESCRIPTOr 
	clear register descriptor
***********************************/
void  clear_desc( int r )		{ rdesc[r].name = NULL ; } 


void  cg_instr( TAC *c ) {

        switch( c->op )
        {
        case TAC_UNDEF:
		
                return ;

        case TAC_ADD:

                cg_bin( "addl", c->varA, c->varB, c->varC ) ;
                return ;

        case TAC_SUB:

                cg_bin( "subl", c->varA, c->varB, c->varC ) ;
                return ;

        case TAC_MUL:

                cg_bin( "imull", c->varA, c->varB, c->varC ) ;
                return ;

        case TAC_DIV:

                cg_bin( "idivl", c->varA, c->varB, c->varC ) ;
                return ;

        case TAC_NEG:
					
	sprintf( (char*) zero, "%d", 0 );
                cg_bin( "subl", c->varA, mkconst( (char*) zero ), c->varB ) ;
                return ;
				
       
	case TAC_ARRAY:
				
		cg_array( c->varA, c->varB, c->varC );
		return;
				
       	case TAC_COPY:
		
		if( c->varA->token_type2 == T_ARRAY )
			c->varA->index = c->varC->val;
					
		cg_copy( c->varA, c->varB ) ;
                	return ;

       	 case TAC_GOTO:

               	 fprintf( file_out, "\tjmp    L%d", c->varA->val );
                	return ;
				
	case TAC_EQUAL:
		
		cg_cond( "je", c->varA->val, c->varB, c->varC );
		return;

	case TAC_NE:
		
		cg_cond( "jne", c->varA->val, c->varB, c->varC );
		return;				;
				
	case TAC_GT:
		
		cg_cond( "jg", c->varA->val, c->varB, c->varC );
		return;

	case TAC_GTE:
		
		cg_cond( "jge", c->varA->val, c->varB, c->varC );
		return;
				
	case TAC_LT:
		
		cg_cond( "jl", c->varA->val, c->varB, c->varC );
		return;
				
	case TAC_LTE:
		
		cg_cond( "jle", c->varA->val, c->varB, c->varC );
		return;				
				
        case TAC_IFZ:

				sprintf( (char*) zero, "%d", 0 );
                cg_cond( "je", c->varA->val, mkconst( (char*) zero ), c->varB ) ;
                return ;

        case TAC_IFNZ:

                return ;

        case TAC_ARG:

                cg_arg( c->varA ) ;
				return ;

        case TAC_CALL:

                cg_call( c->varB ) ;
                return ;

        case TAC_RETURN:

                cg_return( c->varA ) ;
                return ;
				
		case TAC_PRINT:
				
				fprintf( file_out, "\tpushl	$.LC0" );
				fprintf( file_out, "\t\t# push string onto stack\n" );
				fprintf( file_out, "\tcall     printf" );
				fprintf( file_out, "\t\t# call printf function\n" );
				return ;
				
		case TAC_PRINT_STRING:
				
				fprintf( file_out, "\tpushl	$.LC1" );
				fprintf( file_out, "\t\t# push string onto stack\n" );
				fprintf( file_out, "\tcall     printf" );
				fprintf( file_out, "\t\t# call printf function\n" );
				return ;
				
        case TAC_LABEL:
		
				flush_all() ;
                fprintf( file_out, "L%d:\n", c->varA->val ) ;
                return ;
				
		case TAC_PRINTS:
		
				fprintf( file_out, ".LC%d: \n", c->varA->string_num  );
				fprintf( file_out, "\t.string %s\n", c->varA->text );
				fprintf( file_out, "\t.text\n" );
				return;
		
		case TAC_ARG_STRING:
		
				fprintf( file_out, "\tpushl     $.LC%d\n", c->varA->string_num );
				return;
		
		case TAC_BEGIN_LABEL:

                flush_all() ;
				fprintf( file_out, ".LC0: .string \"\\n%s\\n\"\n", "%d" );
				fprintf( file_out, ".LC1: .string \"\\n%s\\n\"\n", "%s" );
				fprintf( file_out, "\t.text\n" );
				fprintf( file_out, ".globl %s\n", c->varA->text );
                fprintf( file_out, "%s:\n", c->varA->text ) ;
                return ;
				
		case TAC_SBEGIN_LABEL:
		
				//flush_all();
				fprintf( file_out, "\n\n" );
				fprintf( file_out, ".globl %s\n", c->varA->text );
				fprintf( file_out, "%s:\n", c->varA->text );
				return;
				
        case TAC_VAR:

                /* Allocate 4 bytes for this variable to hold an integer on the
                   current top of stack */
				if ( c->varA->token_type1 == T_PARAM ){
					param_offset += 4;
					c->varA->base_ptr_offset = tos - c->varA->offset ;
					
					int br = get_rreg( c->varA );
					
					fprintf( file_out, "\tmovl %d(%s), %s\n", param_offset, "%ebp", get_reg_string( br ) );
					
					insert_desc( br, c->varA, MODIFIED ) ;
					
					return;				
				}
				
				if ( c->varA->token_type2 == T_TEMP )	{
					fprintf( file_out, "\tsubl     $4, %s", "%esp" );
					fprintf( file_out, "\t\t# add 4 to stack pointer for temp variable\n" );
					
					tos+=4;
					c->varA->base_ptr_offset = tos;
					
					
					return;
				}
				
				c->varA->base_ptr_offset = tos - c->varA->offset ;
                
                return ;

        case TAC_BEGINFUNC:				
					
				fprintf( file_out, "\tpushl	%s", "%ebp") ;
				fprintf( file_out, "\t\t# push base pointer onto stack\n" );
				
				fprintf( file_out, "\tmovl	%s, %s", "%esp", "%ebp" ) ;
				fprintf( file_out, "\t# move stack pointer address into base pointer\n" );
				
                fprintf( file_out, "\tsubl 	$%d, %s", VAR_OFF, "%esp" ) ;
				fprintf( file_out, "\t# subtract 8 from stack point to be used for linking\n" );
				
				tos = VAR_OFF;
				param_offset = 4;
				
				/*if function has variables */
				if( c->varA->parent->width > 0 ) {
					fprintf( file_out, "\tsubl 	$%d, %s", c->varA->parent->width, "%esp" ) ;
					fprintf( file_out, "\t# subtract width of table for variables\n" );
					
					tos = tos + c->varA->parent->width;
					}
				
                return ;
				
		case TAC_SBEGIN_FUNC:
		
				fprintf( file_out, "\tpushl	%s", "%ebp") ;
				fprintf( file_out, "\t\t# push base pointer onto stack\n" );
				
				fprintf( file_out, "\tmovl	%s, %s", "%esp", "%ebp" ) ;
				fprintf( file_out, "\t# move stack pointer address into base pointer\n" );
				
                fprintf( file_out, "\tsubl 	$%d, %s", VAR_OFF, "%esp" ) ;
				fprintf( file_out, "\t# subtract 8 from stack point to be used for linking\n" );
				
				tos = VAR_OFF;
				
				param_offset = 4;
				
				/*if function has variables */
				printf( "\nName: %s \n", c->varA->text );
				
				if( c->varA->nested_table->width > 0 ) {
					fprintf( file_out, "\tsubl 	$%d, %s", c->varA->nested_table->width, "%esp" ) ;
					fprintf( file_out, "\t# subtract width of table for variables\n" );
					
					tos = tos + c->varA->nested_table->width;
					}
				
                return ;

        case TAC_ENDFUNC:

                /* At the end of the function we put in an implicit return
                   instruction. */
				fprintf( file_out, "\tleave\n") ;
				fprintf( file_out, "\tret\n" );
				  

                /* cg_return( NULL ) ;   Not using right now. */
                return ;
				
		case TAC_ENDSFUNC:

				fprintf( file_out, "\tleave\n") ;
				fprintf( file_out, "\tret\n" );

                return ;

        default:

                /* Don't know what this one is */

                error( "unknown TAC opcode to translate" ) ;
                return ;
        }

} 

void  cg_bin( char *op, SYMB *a, SYMB *b, SYMB *c ) {
		
        int  br = get_rreg( b ) ;        /* Result register */
        int  cr = get_areg( c, br ) ;    /* Second argument register */
		
		if ( c->token_type2 == T_CONSTANT || b->token_type2 == T_CONSTANT ) {
			fprintf( file_out, "\t%s     %s, %s", op, get_reg_string ( cr ), get_reg_string ( br ) ) ;
			fprintf( file_out, "\t\t# add constant and b and store in c \n" );
		}
		
		else {
			fprintf( file_out, "\t%s     %s, %s", op, get_reg_string ( cr ), get_reg_string ( br ) ) ;
			fprintf( file_out, "\t\t# add c and b and store in c \n" );
		}
		
		fprintf( file_out, "\tmovl     %s, %d(%s)", get_reg_string ( br ), 0-a->base_ptr_offset, "%ebp");
		fprintf( file_out, "\t\t# store addition in a \n" );
		

        /* Delete c from the descriptors and insert a */

        clear_desc( br ) ;
        insert_desc( br, a, MODIFIED ) ;

}

void  cg_copy( SYMB *a, SYMB *b ) {
			   
		/* if b is a constant than skip */
		int br, ar;
		
		if ( b->token_type2 == T_CONSTANT ){
			
			if ( a->token_type2 == T_ARRAY ) {
			
				br = get_rreg( b ) ;        /* Load b into a register */
				
				int bpo;
				bpo = 0 - ( a->base_ptr_offset - a->index );
				
				fprintf( file_out, "\tmovl     %s, %d(%s)", get_reg_string( br ), bpo, "%ebp" );
				fprintf( file_out, "\t\t# move register into variable \n" );
			
			}
			
			else {
				br = get_rreg( b ) ;        /* Load b into a register */
				fprintf( file_out, "\tmovl     %s, %d(%s)", get_reg_string( br ), 0-a->base_ptr_offset, "%ebp" );
				fprintf( file_out, "\t\t# move value in register to appropriate pos on stack \n" );
			}
		}
		
		else if ( b->token_type2 == T_FUNC || b->token_type2 == T_NONLOCAL){
			br = get_rreg( b ) ;
			fprintf( file_out, "\tcall     %s\n", b->text );
			fprintf( file_out, "\tmovl     %s, %d(%s)", "%eax", 0-a->base_ptr_offset, "%ebp" );
			fprintf( file_out, "\t\t# move return value into register \n" );
			
		}
		
			
		else {
			br = get_rreg( b );
			fprintf( file_out, "\tmovl     %s, %d(%s)", get_reg_string( br ), 0-a->base_ptr_offset, "%ebp" );
			fprintf( file_out, "\t\t# move argument into destination \n" );
			
		}

         /* Indicate a is there */
		insert_desc( br, a, MODIFIED ) ;
		
}     

void  cg_cond( char *op, int l, SYMB* a, SYMB* b)   {

        spill_all() ;

        int br = get_rreg( b );
		
		int ar = get_areg( a, br );
		
		fprintf( file_out, "\tcmp     %s, %s", get_reg_string( br ), get_reg_string( ar ) );
		fprintf( file_out, "\t\t# compare two registers: if 0 then jump\n" );
		
		fprintf( file_out, "\t%s     L%d",op, l );
		fprintf( file_out, "\t\t# jump \n" );
		
		return;	

}

void  cg_arg( SYMB *a ) {
        
		if( a->token_type1 == T_CONSTANT ) {
			fprintf( file_out, "\tpushl     $%s\n", a->text) ;
			next_arg += 4;
			return;
		}

		fprintf( file_out, "\tpushl     %d(%s)\n", 0-a->base_ptr_offset, "%ebp" ) ;
        next_arg += 4 ;

}   

void  cg_call( SYMB *res ) {

        flush_all() ;
		fprintf( file_out, "\tcall     %s\n", res->text ) ;
        next_arg = 0 ;
}

void cg_array( SYMB* temp, SYMB* index, SYMB* array ) {

	int br;
	
	br = get_rreg( temp );
	
	int bpo = 0 - (   array->base_ptr_offset - index->val );
	
	fprintf( file_out, "\tmovl     %d(%s), %s\n", bpo, "%ebp", get_reg_string ( br ));
	fprintf( file_out, "\tmovl     %s, %d(%s)\n", get_reg_string ( br ), 0 - temp->base_ptr_offset, "%ebp" );
	
	insert_desc( br, temp, MODIFIED ) ;	

}

void cg_return( SYMB* ret ){

	int br;
	
	br = get_rreg( ret );
	
	fprintf( file_out, "\tmovl     %s, %s", get_reg_string( br ), "%eax" );
	fprintf( file_out, "\t\t# move variable to eax\n" );
	fprintf( file_out, "\tleave \n");
	fprintf( file_out, "\tret\n" );

}
