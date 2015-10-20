/*Bryan Martin Tostado
 * created 25/march/2014
 *
 */

#ifndef __CODGEN__
  #define __CODGEN__

#define PL0_MAIN_FIX  //the problem with having to declair main as l
#define PL0_MATRIX_FIX //pl0 virtual machine needs large matrxi sizes or else it wont save eleements of matrix right

#define MAX_PL0_CODEGEN 10000
#define MAX_PL0_SYMTAB 1000
extern char *outputFileName;
extern int code_ptr;
extern int symtab_ptr;

struct codegen_code_t
{
  char* mnemo;
  char* dir1;
  char *dir2;
}; 


/* inserts symbol for a PL0 codegen
 * returns: nothing
 */
void insSymTabPL0(char *name_, char class_, char type_, int dim1_, int dim2_);

/*Inserts label into symtab
 */
void insLabelSymTabPL0(char *name_);

/* creates code for a PL0 stack machine
 * returns: nothing
 */
void codeGenPL0(char *mnemo_, char *dir1_,char *dir2);

/*  It just places code[idx] = code[idx-i]
 */
void repeatLastCodeGenPL0(int );

/* returns the struct with the mneno, dir1,dir2 of the last-i code generated*/
struct codegen_code_t previousCodeGenPL0();

/* insert the code into the specified index*/
void codeGenPL0Index(char *mnemo_, char *dir1_,char *dir2_,int i);

/* flops "inverts" all code from th flop_index to the code_ptr -1i (last code inserted) */
void codeGenPL0Flop(int flop_index);

/* retunrs the string of the name of the next numberd label in the format _E# */
char* nextLabel();

/* dumps the symtab and code generated to a file*/
void writePL0();

/* frees up all the structs used when creating the PL0 code*/
void codegenFreePL0();

/* insert the pointers that SHOULD be freed that were inserted into the symtab
 * most of the strings inserted into symtab are raw strings, so this is necesary
 */
void  codegenPL0PtrFreeList(char *string);

#endif /*__CODGEN__*/
