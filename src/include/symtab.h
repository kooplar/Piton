/* Bryan Martin Tostado
 *
 * creation: 13,march 2014
 *
 * Description:
 *  hash table for simbols. The first table will have all globals. Each descent into a block creates another hash table. When looking for a variable, look in the the last hash table, then work your way up (local...global). Symbol table is a stack wich pusp,pop functions.
 *
 *  Notes:
 *    
 *
 */


#ifndef __TABSIM__
  #define __TABSIM__

#include "lexer.h"

#define HASHSIZE 20 

//PL0
extern int found_in_depth; //to know if you need to concat func name to var or not
//\PL0


/* entry into the symbol table (hastable). nextSymbolis to bucket duplicate hash values*/
typedef struct symtabEntry symtabEntry_t;
struct symtabEntry{
 int dim1; //0 for funcs
 int dim2; //0 for funcs
 char* lexeme;
 int type; //return type for funcs
 int constant; //0 for funcs
 char* value; // NULL for funcs
 symtabEntry_t *nextSymbol; // bucket
};

/* the level (block/scope) to which these symbols correspond */
struct symtabLevel{
  struct symtabLevel *next;
  struct symtabLevel *prev;
  symtabEntry_t *entry[HASHSIZE];
  int depth;
};
typedef struct symtabLevel symtabLevel_t;


extern symtabLevel_t *symtabHead;
extern symtabLevel_t *symtabCurrent;


/* pushSymtab
 *
 *  Description:
 *   Creates a new symtab (which is really just a hashtable). This symtab is linked (doubly linked list) to the previous 
 *   symtab created (or whatever the value of currentSymtab is). If this is the first call to newSymtab, the pointer 
 *   symtabHead is set to this symtab also. All keywords are set into this symtab.
 *  Notes:
 *  Returns:
 *   0 on error, the depth of the symtab otherwise
 */
int pushSymtab();


/* pushSymtab
 *
 *  Description:
 *   Makes you go one symtab scope lower and frees every symbol entry in the current scope.
 *  Notes:
 *  Returns:
 *   0 on error (if symtab current is already head for example), the depth of the symtab otherwise
 */
int popSymtab();

/* symtabInsert
 *
 *  Description:
 *   Inserts the token into the symtab
 *  Notes:
 *   If token = keyword it is NOT inserted
 *  Returns:
 *    ERR or 1 on OK 
 *   
 * */
int symtabInsert(char* lexeme, symtabLevel_t *symtab,int dim1,int dim2,int type,int constant,char *value);

/* symtabInsertHead
 * 
 *  Description:
 *   inserts token into the current symtab (defiend by symtabCurrent)
 *  Notes:
 *   calls symtabInsert with  symtabLevel = symtabHead 
 *  Returns:
 *   returns call to symtabInsert.
 */
int symtabInsertHead(char *lexeme,int dim1,int dim2,int type,int constant,char *value);

/* symtabInsertCurrent
 *
 *  Description:
 *   inserts token into the current symtab (defiend by symtabCurrent)
 *  Notes:
 *   calls symtabInsert with  symtabLevel = symtabCurrent
 *  Returns:
 *   returns call to symtabInsert.
 */
int symtabInsertCurrent(char *lexeme,int dim1,int dim2,int type,int constant,char *value);

/* symtabLookupCurrent
 * 
 *
 */
symtabEntry_t* symtabLookupHead(char *symbol);

/* symtabLookupCurrent
 *
 *
 */
symtabEntry_t* symtabLookup(char *symbol, symtabLevel_t *symtab);

/* symtabLookupCurrent
 *
 *
 */
symtabEntry_t* symtabLookupCurrent(char *symbol);

/* hash
 *
 *  Description:
 *   returns a hash value for the string provided
 *  Notes:
 *  Returns:
 *   The hash value generated
 */
int hash(const char *);

/*frees every symtab that is still allocated (including head)*/
void freeSymtabs();


 




#endif /* __TABSIM__ */
