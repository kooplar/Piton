/* Bryan Martin Tostado
 * 
 * creation: April 2014
 *
 * Description:
 *  entry functions to print error messages for lex,semantic,syntactic errors. The counter vars are used to be able to stop execution at any point (and print the total errors recoverd.  
 *
 *  Notes:
 *   Some errors are recoverable (usualy errors that dont affect global scope) this is why the compilation doesnt terminate on any error encounterd.
 *     Its up to the syntactic/semantic analisis to determine if the error is recoverabel or not. Please note that the program should ALWAYS exit
 *      function cleanExit(). 
 *
 */

#ifndef __ERRORS__
  #define __ERRORS__

#define SUCCESS 1
#define FAILURE 0

/*vars*/
extern int lexErrors;
extern int syntaxErrors;
extern int semErrors;

/*FUNCS*/

//cleanups all memory and exits
void cleanExit(int status,char *msg);

//returns number of errors found so far
int errorCount();

//lex errors
/* -----------------------------------------------------*/
void pLexSym(char *msg,char symbol);
void pLexSymString(char *msg,char* symbol);
//prints the line num and just the string
void pLexString(char *msg);


//syntax errors
/* ---------------------------------------------------------*/
void pSynExpected( int expected,int got);
void pSyn2Expected( int expected1,int expected2,int got);
void pSynExpectedString( char *expected, int got);
//print String 
void pSynRawString(char *msg);
//printes line and char number and the string
void pSynString(char *msg);
//prints the variable and the mesg ( example, var x should be dimensiond but is).


//semantic errors
/* ----------------------------------------------------------*/
void pSemString(char *msg);
void pSemRawString(char *msg);
void pSemDupeSymbol(char *msg);
void pSemUndecSym(char *symbolName);
void pSemTypeConflict(int operator_1, int operand, int operator_2,long expr_start,long expr_end);
void pSemUndecFunc(char *func);
void pSemVar(char *var, char *msg);
void pSemExpectedType(int expected,int got, long expr_start,long expr_end,char *m);
void pSemFunc(char* funcName, char *msg);


#endif /* __ERRORS__ */
